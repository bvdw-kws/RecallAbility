// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "Utility/Ability/RecallAbilityUtils.h"

#include "Ability/RecallAbilityCommandTypes.h"
#include "Ability/RecallAbilityConditionTypes.h"
#include "Ability/RecallAbilityExecutionTypes.h"
#include "Data/Ability/RecallAbilityAsset.h"
#include "Simulation/Ability/RecallAbilityFragments.h"
#include "Simulation/GameplayTag/RecallGameplayTagFragments.h"

namespace Recall::Ability::Utils
{

bool EvaluateAbilityCondition(const FInstancedStruct& Condition, const FRecallAbilityExecutionContext& Context)
{
	if (!Condition.IsValid())
	{
		return true;
	}

	const FRecallAbilityConditionBase* ConditionPtr = Condition.GetPtr<FRecallAbilityConditionBase>();
	if (ConditionPtr == nullptr)
	{
		return true;
	}

	return ConditionPtr->Evaluate(Context);
}
	
void ProcessAbilityCommandSequence(const TObjectPtr<const URecallAbilityAsset>& Ability,
	const FRecallAbilityExecutionContext& AbilityExecutionContext, ERecallAbilityExecutionEvent Event)
{
	if (!Ability)
	{
		return;
	}	
	
	for (const FInstancedStruct& ActionCommand : Ability->Commands)
	{
		const FRecallAbilityCommand* CommandPtr = ActionCommand.GetPtr<FRecallAbilityCommand>();		
		if (CommandPtr == nullptr || !CommandPtr->EvaluateCondition(AbilityExecutionContext))
		{
			continue;
		}
			
		CommandPtr->Execute(AbilityExecutionContext, Event);
	}
}

bool NavigateToAnimationSegmentByIdentifier(
	const FRecallAbilityExecutionContext& AbilityExecutionContext,
	FRecallAbilityAnimationFragment& AnimationFragment, const FName& Name)
{
	if (Name.IsNone())
	{
		return false;
	}
	
	FRecallAbilityFragment& AbilityFragment = AbilityExecutionContext.AbilityFragment;
	
	// Locate animation segment matching the specified identifier
	const int32 SectionIndex = AbilityFragment.CurrentAbility->AnimationSections.IndexOfByPredicate (
		[Name](const FRecallAbilityAnimationSection& Section)
	{
		return Section.Name == Name;
	});
	
	if (SectionIndex == INDEX_NONE)
	{
		return false;
	}

	AnimationFragment.CurrentSectionIndex = SectionIndex;

	int32 StartFrame = 0;

	const FRecallAbilityAnimationSection& AnimationSection = AbilityFragment.CurrentAbility->AnimationSections[SectionIndex];
	const TObjectPtr<UObject> CurrentAnimationAsset = AnimationSection.GetAnimationAsset();

	if (CurrentAnimationAsset && AnimationFragment.AnimationAsset != CurrentAnimationAsset)
	{
		AnimationFragment.AnimationAsset = CurrentAnimationAsset;
		AnimationFragment.bChangeAnimation = true;
	}

	// Adjust frame position to align with target animation segment start
	for(int32 Index = 0; Index < SectionIndex; Index++)
	{
		StartFrame += AbilityFragment.CurrentAbility->AnimationSections[Index].Duration;
	}

	AbilityFragment.SetCurrentFrame(StartFrame);
		
	return true;
}

void ExitCurrentAbility(const FRecallAbilityExecutionContext& AbilityExecutionContext)
{
	ProcessAbilityCommandSequence(
		AbilityExecutionContext.AbilityFragment.CurrentAbility, AbilityExecutionContext, ERecallAbilityExecutionEvent::OnExit);
				
	if (AbilityExecutionContext.AbilityFragment.CurrentAbility)
	{
		if (AbilityExecutionContext.TagsFragmentPtr != nullptr)
		{
			AbilityExecutionContext.TagsFragmentPtr->GameplayTagCountMap.RemoveTags(
				AbilityExecutionContext.AbilityFragment.CurrentAbility->AbilityTags);
		}
	}

	AbilityExecutionContext.AbilityFragment.CurrentAbility = nullptr;
	AbilityExecutionContext.AbilityFragment.ResetAbilityFrame();
}

void ProcessAbilitySequence(
	const FRecallAbilityExecutionContext& AbilityExecutionContext, FRecallAbilityAnimationFragment& AnimationFragment)
{
	FRecallAbilityFragment& AbilityFragment = AbilityExecutionContext.AbilityFragment;
	
	// Early exit if no transition is pending
	if (!AbilityFragment.bTransitionTriggered)
	{
		return;
	}

	// Ensure valid next ability exists for transition
	checkf(AbilityFragment.NextAbility, TEXT("Ability transition requires valid target ability"));

	// Clear transition flag to prevent repeated processing
	AbilityFragment.bTransitionTriggered = false;

	// Verify transition prerequisites are satisfied
	if (!EvaluateAbilityCondition(AbilityFragment.NextAbility->Condition, AbilityExecutionContext))
	{
		// Reset queued transition data on validation failure
		AbilityFragment.NextAbility = nullptr;
		AbilityFragment.NextSection = NAME_None;
		return;
	}
	
	
	// Capture previous ability state for transition logic
	const TObjectPtr<const URecallAbilityAsset> FormerAbility = AbilityFragment.CurrentAbility;

	// Terminate existing ability execution
	ExitCurrentAbility(AbilityExecutionContext);
	
	// Check if this is the entity's first ability activation
	const bool bFirstActivation = !FormerAbility;
	
	// Activate the queued ability
	AbilityFragment.CurrentAbility = AbilityFragment.NextAbility;
	
	// Configure animation blending behavior based on transition type
	if (bFirstActivation)
	{
		// Skip blending for initial ability activation
		AnimationFragment.bForceResetBlend = true;
	}
	else
	{
		// Enable smooth blending between abilities
		AnimationFragment.bForceResetBlend = false;
	}

	// Override blending when explicitly disabled
	if(AbilityFragment.bIgnoreBlend)
	{
		AnimationFragment.bForceResetBlend = true;
	}

	// Execute ability entry operations
	ProcessAbilityCommandSequence(AbilityFragment.CurrentAbility, AbilityExecutionContext, ERecallAbilityExecutionEvent::OnEnter);

	// Apply gameplay tags associated with the new ability
	if (AbilityExecutionContext.TagsFragmentPtr != nullptr)
	{
		AbilityExecutionContext.TagsFragmentPtr->GameplayTagCountMap.AddTags(AbilityFragment.CurrentAbility->AbilityTags);
	}

	// Initialize animation state for new ability
	AnimationFragment.ResetOnEnterAbility();

	// Clear previous animation reference for clean transition
	AnimationFragment.AnimationAsset = nullptr;

	// Configure initial animation segment if available
	if (AbilityFragment.CurrentAbility->AnimationSections.Num() > 0)
	{
		const FRecallAbilityAnimationSection& InitialSegment = AbilityFragment.CurrentAbility->AnimationSections[0];
		if (const TObjectPtr<UObject> SegmentAnimation = InitialSegment.GetAnimationAsset())
		{
			AnimationFragment.AnimationAsset = SegmentAnimation;
		}
	}
	
	// Flag animation system for asset update
	AnimationFragment.bChangeAnimation = true;

	// Request montage activation for first-time abilities
	if (bFirstActivation)
	{
		AnimationFragment.bPlayNewAnimationMontage = true;
	}

	// Process any pending animation segment navigation
	NavigateToAnimationSegmentByIdentifier(AbilityExecutionContext, AnimationFragment, AbilityFragment.NextSection);

	// Clear transition data to prevent reprocessing
	AbilityFragment.NextAbility = nullptr;
	AbilityFragment.NextSection = NAME_None;
	
	// Execute initial ability tick
	ProcessAbilityCommandSequence(AbilityFragment.CurrentAbility, AbilityExecutionContext, ERecallAbilityExecutionEvent::OnTick);
}

} // namespace Recall::Ability::Utils
