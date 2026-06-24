// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallAbilityTasks.h"

#include "RecallSignalSubsystem.h"
#include "Data/Ability/RecallAbilityCollectionAsset.h"
#include "Data/Inventory/RecallInventoryItemAsset.h"
#include "Simulation/Ability/RecallAbilityFragments.h"
#include "Simulation/Inventory/RecallInventoryFragments.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"
#include "Simulation/Ability/RecallAbilityChainFragments.h"
#include "Simulation/Ability/RecallAbilityChainSignalTypes.h"
#include "StateTree/RecallStateTreeExecutionContext.h"
#include "System/Inventory/RecallItemSubsystem.h"

static TObjectPtr<const URecallAbilityCollectionAsset> GetAbilityCollection(const UWorld* World,
	const FRecallAbilityFragment& AbilityFragment, bool bUseSelectedEquipSlot,
	const FRecallEquipmentFragment* EquipmentFragment)
{
	if (bUseSelectedEquipSlot)
	{
		FGameplayTag ItemTag;
		if (EquipmentFragment != nullptr && EquipmentFragment->Equipment.GetEquipmentSlot(
				EquipmentFragment->SelectedEquipSlot, ItemTag))
		{
			const URecallItemSubsystem& ItemSystem = URecallItemSubsystem::GetRef(World);
			if (const TObjectPtr<const URecallInventoryItemAsset> ItemAsset = ItemSystem.GetItemAsset(ItemTag))
			{
				return Cast<URecallAbilityCollectionAsset>(ItemAsset->AbilityCollection);
			}
		}
		return nullptr;
	}
	else
	{
		return AbilityFragment.AbilityCollection;
	}
}

//----------------------------------------------------------------------//
// FRecallAbilityTask
//----------------------------------------------------------------------//
bool FRecallAbilityTask::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(AbilityFragmentHandle);
	Linker.LinkExternalData(EquipmentFragmentHandle);
	return true;
}

EStateTreeRunStatus FRecallAbilityTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.bFinishedAbility = false;
	
	const TObjectPtr<URecallAbilityAsset> AbilityAsset = GetAbilityAsset(Context);
	if (!ensureAlwaysMsgf(AbilityAsset,
		TEXT("%hs Ability is not set"), __FUNCTION__))
	{
		return EStateTreeRunStatus::Failed;
	}

	FRecallAbilityFragment& AbilityFragment = Context.GetExternalData(AbilityFragmentHandle);
	AbilityFragment.TransitionToAbility(AbilityAsset, bIgnoreBlend, Section, InstanceData.Targets);
	
	return Super::EnterState(Context, Transition);
}

EStateTreeRunStatus FRecallAbilityTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	
	const FRecallAbilityFragment& AbilityFragment = Context.GetExternalData(AbilityFragmentHandle);
	const TObjectPtr<URecallAbilityAsset> AbilityAsset = GetAbilityAsset(Context);
	
	// Wait until the ability is done.
	if (!InstanceData.bFinishedAbility && AbilityFragment.CurrentAbility != AbilityAsset)
	{
		InstanceData.bFinishedAbility = true;
		Context.BroadcastDelegate(InstanceData.OnAbilityFinishedDelegate);
		
		if (bSucceedOnFinish)
		{
			return EStateTreeRunStatus::Succeeded;
		}
	}

	return Super::Tick(Context, DeltaTime);
}

TObjectPtr<URecallAbilityAsset> FRecallAbilityTask::GetAbilityAsset(FStateTreeExecutionContext& Context) const
{
	switch (AbilitySource)
	{
	case ERecallAbilityTaskSource::Asset:
		return Ability;

	case ERecallAbilityTaskSource::Collection:
		{
			const FRecallAbilityFragment& AbilityFragment = Context.GetExternalData(AbilityFragmentHandle);
			const FRecallEquipmentFragment* EquipmentFragmentPtr = Context.GetExternalDataPtr(EquipmentFragmentHandle);
			
			if (const TObjectPtr<const URecallAbilityCollectionAsset> AbilityCollection = GetAbilityCollection(
					Context.GetWorld(), AbilityFragment, bUseSelectedEquipSlot, EquipmentFragmentPtr))
			{
				return AbilityCollection->Abilities.FindRef(AbilityTag);
			}
		}
		return nullptr;

	default:
		unimplemented();
		return nullptr;
	}
}

//----------------------------------------------------------------------//
// FRecallAbilityChainTask
//----------------------------------------------------------------------//
FRecallAbilityChainTask::FRecallAbilityChainTask()
	: Super()
{
}

bool FRecallAbilityChainTask::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(AbilityFragmentHandle);
	Linker.LinkExternalData(AbilityChainFragmentHandle);
	Linker.LinkExternalData(AbilityChainInputFragmentHandle);
	Linker.LinkExternalData(EquipmentFragmentHandle);
	return true;
}

EStateTreeRunStatus FRecallAbilityChainTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (!InstanceData.AbilityChain.IsValid())
	{
		return EStateTreeRunStatus::Failed;
	}
	
	const TObjectPtr<const URecallAbilityCollectionAsset> AbilityCollectionAsset = GetAbilityCollectionAsset(
		Context);
	if (!AbilityCollectionAsset)
	{
		return EStateTreeRunStatus::Failed;
	}

	// Start the ability chain if not already queued or active.
	FRecallAbilityChainFragment& AbilityChainFragment = Context.GetExternalData(AbilityChainFragmentHandle);
	if (AbilityChainFragment.Start(AbilityCollectionAsset))
	{
		FRecallStateTreeExecutionContext& RecallContext = static_cast<FRecallStateTreeExecutionContext&>(Context);
		RecallContext.GetSignalSystem().SignalEntity(
			Recall::AbilityChain::Signals::Start, RecallContext.GetEntity());
	}	

	// Push the ability chain input.
	FRecallAbilityChainInputFragment& AbilityChainInputFragment = Context.GetExternalData(AbilityChainInputFragmentHandle);
	AbilityChainInputFragment.PushAbilityChain(InstanceData.AbilityChain);
	AbilityChainInputFragment.Direction = InstanceData.Direction;
	
	AbilityChainFragment.TargetEntities.Reset(InstanceData.TargetEntities.Num());
	for (const FMassEntityHandle& TargetEntity : InstanceData.TargetEntities)
	{
		if (TargetEntity.IsSet())
		{
			AbilityChainFragment.TargetEntities.Add(TargetEntity);
		}
	}

	return FRecallStateTreeTaskBase::EnterState(Context, Transition);
}

EStateTreeRunStatus FRecallAbilityChainTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	return EStateTreeRunStatus::Succeeded;
}

TObjectPtr<const URecallAbilityCollectionAsset> FRecallAbilityChainTask::GetAbilityCollectionAsset(
	FStateTreeExecutionContext& Context) const
{
	switch (AbilitySource)
	{
	case ERecallAbilityTaskSource::Asset:
		return AbilityCollection;

	case ERecallAbilityTaskSource::Collection:
		{
			const FRecallAbilityFragment& AbilityFragment = Context.GetExternalData(AbilityFragmentHandle);
			const FRecallEquipmentFragment* EquipmentFragmentPtr = Context.GetExternalDataPtr(EquipmentFragmentHandle);
			
			return GetAbilityCollection(Context.GetWorld(), AbilityFragment, bUseSelectedEquipSlot, EquipmentFragmentPtr);
		}

	default:
		unimplemented();
		return nullptr;
	}
}
