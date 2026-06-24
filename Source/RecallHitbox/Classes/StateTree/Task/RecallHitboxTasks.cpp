// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallHitboxTasks.h"

#include "Simulation/Inventory/RecallInventoryFragments.h"
#include "StateTree/RecallStateTreeExecutionContext.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"
#include "Simulation/Hitbox/RecallHitTypes.h"

//----------------------------------------------------------------------//
// FRecallLocalizeHitTask
//----------------------------------------------------------------------//
bool FRecallLocalizeHitTask::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(EquipmentConstSharedFragmentHandle);
	Linker.LinkExternalData(EquipmentFragmentHandle);
	return true;
}

TArray<FRecallLocalizedHit> FRecallLocalizeHitTask::ExtractLocalizedHit(FStateTreeExecutionContext& Context, const TArray<FRecallHit>& Hits) const
{
	TArray<FRecallLocalizedHit>  Results;
	Results.SetNum(Hits.Num());

	const FRecallEquipmentConstSharedFragment& EquipmentConstSharedFragment = Context.GetExternalData(EquipmentConstSharedFragmentHandle);
	const FRecallEquipmentFragment& EquipmentFragment = Context.GetExternalData(EquipmentFragmentHandle);
	
	TArray<FGameplayTag> EquipSlots;
	EquipmentConstSharedFragment.Settings.EquipmentSlots.GetGameplayTagArray(EquipSlots);
	
	for (int32 HitIndex = 0; HitIndex < Hits.Num(); HitIndex++)
	{
		const FRecallHit& Hit = Hits[HitIndex];
		
		FRecallLocalizedHit& LocalizedHit = Results[HitIndex];
		LocalizedHit.Location = Hit.Location;

		for (const FGameplayTag& EquipSlotTag : EquipSlots)
		{
			if (EquipmentFragment.Equipment.IsEmptySlot(EquipSlotTag))
			{
				continue;
			}
		
			// TODO: Find nearest equip slot
			LocalizedHit.EquipSlot = EquipSlotTag;
			break;
		}
	}
	
	return Results;
}

EStateTreeRunStatus FRecallLocalizeHitTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);	

	// Gather our localized hits.
	const TArray<FRecallLocalizedHit> LocalizedHits = ExtractLocalizedHit(Context, { InstanceData.Hit });	

	// Fail if we did not find any hit to localize.
	if (LocalizedHits.Num() == 0)
	{
		return EStateTreeRunStatus::Failed;
	}

	// Set our localized hits.
	InstanceData.LocalizedHit = LocalizedHits[0];
	
	if (bSucceedOnDone)
	{
		return EStateTreeRunStatus::Succeeded;
	}
	
	return Super::EnterState(Context, Transition);
}

void FRecallLocalizeHitTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	return Super::ExitState(Context, Transition);
}

EStateTreeRunStatus FRecallLocalizeHitTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	return Super::Tick(Context, DeltaTime);
}
