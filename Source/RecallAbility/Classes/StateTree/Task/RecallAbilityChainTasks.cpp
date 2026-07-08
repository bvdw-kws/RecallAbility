// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallAbilityChainTasks.h"

#include "Data/Ability/RecallAbilityAsset.h"
#include "Data/Ability/RecallAbilityCollectionAsset.h"
#include "RecallSignalSubsystem.h"
#include "Physics/RecallPhysicsObjects.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"
#include "Simulation/Ability/RecallAbilityChainFragments.h"
#include "Simulation/Ability/RecallAbilityChainSignalTypes.h"
#include "Simulation/Ability/RecallAbilityFragments.h"
#include "Simulation/Physics/RecallPhysicsBodyFragment.h"
#include "StateTree/RecallStateTreeExecutionContext.h"
#include "System/Physics/RecallPhysicsSubsystem.h"

#define LOCTEXT_NAMESPACE "RecallAbilityChainTasks"

//----------------------------------------------------------------------//
// FRecallAbilityChainExecTask
//----------------------------------------------------------------------//
FRecallAbilityChainExecTask::FRecallAbilityChainExecTask()
	: Super()
{
}

bool FRecallAbilityChainExecTask::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(AbilityFragmentHandle);
	Linker.LinkExternalData(AbilityChainFragmentHandle);
	Linker.LinkExternalData(AbilityChainInputFragmentHandle);
	return true;
}

EStateTreeRunStatus FRecallAbilityChainExecTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	const TObjectPtr<URecallAbilityAsset> AbilityAsset = GetAbilityAsset(Context);
	if (!ensureAlwaysMsgf(AbilityAsset,
		TEXT("%hs Ability is not set: %s"), __FUNCTION__, *AbilityTag.ToString()))
	{
		return EStateTreeRunStatus::Failed;
	}
	
	const FName ActiveStateName = Context.GetActiveStateNames().Last();
	
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (InstanceData.StateName.IsNone())
	{
		InstanceData.StateName = ActiveStateName;
	}
	InstanceData.bCanExit = false;
	
	if (InstanceData.StateName == ActiveStateName)
	{
		const FRecallAbilityChainFragment& AbilityChainFragment = Context.GetExternalData(AbilityChainFragmentHandle);
		
		FRecallAbilityFragment& AbilityFragment = Context.GetExternalData(AbilityFragmentHandle);
		AbilityFragment.TransitionToAbility(AbilityAsset, bIgnoreBlend, Section, AbilityChainFragment.TargetEntities);
	}

	return Super::EnterState(Context, Transition);
}

void FRecallAbilityChainExecTask::ExitState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.StateName = NAME_None;
	InstanceData.bCanExit = false;
	
	FRecallAbilityChainInputFragment& AbilityChainInputFragment = Context.GetExternalData(AbilityChainInputFragmentHandle);
	AbilityChainInputFragment.ClearInputs();
}

EStateTreeRunStatus FRecallAbilityChainExecTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	const TObjectPtr<URecallAbilityAsset> AbilityAsset = GetAbilityAsset(Context);
	if (!AbilityAsset)
	{
		return EStateTreeRunStatus::Failed;
	}

	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.bCanExit = false;
	
	if (InstanceData.StateName == Context.GetActiveStateNames().Last())
	{
		const FRecallAbilityFragment& AbilityFragment = Context.GetExternalData(AbilityFragmentHandle);
		if (AbilityFragment.CurrentAbility == AbilityAsset)
		{
			InstanceData.bCanExit = AbilityFragment.ElapsedFrames >= AbilityAsset->AbilityChainExitFrame;
		}
		else if (bExitOnFinish)
		{
			return EStateTreeRunStatus::Succeeded;
		}
	}
	
	return Super::Tick(Context, DeltaTime);
}

TObjectPtr<URecallAbilityAsset> FRecallAbilityChainExecTask::GetAbilityAsset(FStateTreeExecutionContext& Context) const
{
	const FRecallAbilityChainFragment& AbilityChainFragment = Context.GetExternalData(AbilityChainFragmentHandle);
	if (const TObjectPtr<const URecallAbilityCollectionAsset> AbilityCollection = AbilityChainFragment.GetActiveAbilityCollection())
	{
		return AbilityCollection->Abilities.FindRef(AbilityTag);
	}

	return nullptr;
}

//----------------------------------------------------------------------//
// FRecallAbilityChainDelayTask
//----------------------------------------------------------------------//
bool FRecallAbilityChainDelayTask::Link(FStateTreeLinker& Linker)
{
	return true;
}

EStateTreeRunStatus FRecallAbilityChainDelayTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	const FRecallStateTreeExecutionContext& MassContext = static_cast<FRecallStateTreeExecutionContext&>(Context);
	const FRandomStream& RandomStream = MassContext.GetRandomStream();

	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!bRunForever)
	{
		InstanceData.RemainingTime = RandomStream.FRandRange(
			FMath::Max(0.0f, Duration - RandomDeviation), (Duration + RandomDeviation));

		MassContext.GetSignalSystem().DelaySignalEntity(
			Recall::AbilityChain::Signals::Delay, MassContext.GetEntity(), InstanceData.RemainingTime);
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FRecallAbilityChainDelayTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!bRunForever)
	{
		InstanceData.RemainingTime -= DeltaTime;

		if (InstanceData.RemainingTime <= UE_KINDA_SMALL_NUMBER)
		{
			return EStateTreeRunStatus::Succeeded;
		}
	}

	return EStateTreeRunStatus::Running;
}

#if WITH_EDITOR
FText FRecallAbilityChainDelayTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
	const FInstanceDataType* InstanceData = InstanceDataView.GetPtr<FInstanceDataType>();
	check(InstanceData);

	FText Value = FText::GetEmpty();

	if (const FPropertyBindingPath* RunForeverSourcePath = BindingLookup.GetPropertyBindingSource(FPropertyBindingPath(ID, GET_MEMBER_NAME_CHECKED(FRecallAbilityChainDelayTask, bRunForever))))
	{
		Value = FText::Format(LOCTEXT("ForeverBound", "Forever={0}"),
			BindingLookup.GetPropertyPathDisplayName(*RunForeverSourcePath, Formatting));
	}
	else if (bRunForever)
	{
		Value = LOCTEXT("Forever", "Forever");
	}
	else
	{
		FNumberFormattingOptions Options;
		Options.MinimumFractionalDigits = 1;
		Options.MaximumFractionalDigits = 3;

		FText DurationText = BindingLookup.GetBindingSourceDisplayName(FPropertyBindingPath(ID, GET_MEMBER_NAME_CHECKED(FRecallAbilityChainDelayTask, Duration)), Formatting);
		if (DurationText.IsEmpty())
		{
			DurationText = FText::AsNumber(Duration, &Options);
		}

		FText RandomDeviationText = BindingLookup.GetBindingSourceDisplayName(FPropertyBindingPath(ID, GET_MEMBER_NAME_CHECKED(FRecallAbilityChainDelayTask, RandomDeviation)), Formatting);
		if (RandomDeviationText.IsEmpty()
			&& !FMath::IsNearlyZero(RandomDeviation))
		{
			RandomDeviationText = FText::AsNumber(RandomDeviation, &Options);
		}

		if (RandomDeviationText.IsEmpty())
		{
			Value = DurationText;
		}
		else
		{
			if (Formatting == EStateTreeNodeFormatting::RichText)
			{
				Value = FText::Format(LOCTEXT("DelayValueRich", "{0} <s>\u00B1{1}</>"), // +-
					DurationText,
					RandomDeviationText);
			}
			else
			{
				Value = FText::Format(LOCTEXT("DelayValue", "{0} \u00B1{1}"), // +-
					DurationText,
					RandomDeviationText);
			}
		}
	}

	const FText Format = (Formatting == EStateTreeNodeFormatting::RichText)
		? LOCTEXT("DelayRich", "<b>Delay</> {Time}")
		: LOCTEXT("Delay", "Delay {Time}");

	return FText::FormatNamed(Format,
		TEXT("Time"), Value);
}
#endif // WITH_EDITOR

//----------------------------------------------------------------------//
// FRecallAbilityChainRotateTask
//----------------------------------------------------------------------//
FRecallAbilityChainRotateTask::FRecallAbilityChainRotateTask()
	: Super()
{
	bShouldStateChangeOnReselect = false;
}

bool FRecallAbilityChainRotateTask::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(BodyFragmentHandle);
	Linker.LinkExternalData(PhysicsSystemHandle);
	return true;
}

EStateTreeRunStatus FRecallAbilityChainRotateTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.EnterRotation = InstanceData.Rotation;
	
	URecallPhysicsSubsystem& PhysicsSystem = Context.GetExternalData(PhysicsSystemHandle);
	const FRecallPhysicsBodyFragment& BodyFragment = Context.GetExternalData(BodyFragmentHandle);

	const FRecallPhysicsBodyView Body = PhysicsSystem.GetMutableBody(BodyFragment.BodyHandle);
	if (Body.IsValid())
	{
		FQuat Rotation = FQuat::Identity;
		Body.GetRotation(Rotation);

		Rotation *= InstanceData.EnterRotation.Quaternion();

		Body.SetRotation(Rotation);
	}
	
	return Super::EnterState(Context, Transition);
}

void FRecallAbilityChainRotateTask::ExitState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (bRestoreRotationOnExit)
	{
		URecallPhysicsSubsystem& PhysicsSystem = Context.GetExternalData(PhysicsSystemHandle);
		const FRecallPhysicsBodyFragment& BodyFragment = Context.GetExternalData(BodyFragmentHandle);

		const FRecallPhysicsBodyView Body = PhysicsSystem.GetMutableBody(BodyFragment.BodyHandle);
		if (Body.IsValid())
		{
			FQuat Rotation = FQuat::Identity;
			Body.GetRotation(Rotation);

			Rotation *= InstanceData.EnterRotation.Quaternion().Inverse();

			Body.SetRotation(Rotation);
		}
	}
}

#undef LOCTEXT_NAMESPACE
