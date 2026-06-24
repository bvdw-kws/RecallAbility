// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallAbilityProcessors.h"

#include "Ability/RecallAbilityCommandTypes.h"
#include "Ability/RecallAbilityExecutionTypes.h"
#include "Data/Ability/RecallAbilityAsset.h"
#include "MassExecutionContext.h"
#include "Simulation/Ability/RecallAbilityFragments.h"
#include "Simulation/Ability/RecallAbilityProcessorGroupTypes.h"
#include "Simulation/GameplayTag/RecallGameplayTagFragments.h"
#include "Simulation/Movement/RecallMovementProcessorGroupTypes.h"
#include "Simulation/Transform/RecallTransformFragments.h"
#include "Utility/Ability/RecallAbilityUtils.h"

//----------------------------------------------------------------------//
// URecallAbilityProcessor
//----------------------------------------------------------------------//
URecallAbilityProcessor::URecallAbilityProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = static_cast<int32>(EExtendedProcessorExecutionFlags::All);
	ProcessingPhase = EMassProcessingPhase::StartPhysics;
	ExecutionOrder.ExecuteInGroup = Recall::Ability::ProcessorGroupNames::StartPhysics::Ability;
	ExecutionOrder.ExecuteAfter.Add(Recall::Ability::ProcessorGroupNames::StartPhysics::Animation);
	ExecutionOrder.ExecuteBefore.Add(Recall::Movement::ProcessorGroupNames::StartPhysics::Update);
}

void URecallAbilityProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager)
{
	Super::InitializeInternal(Owner, InEntityManager);
}

void URecallAbilityProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FRecallAbilityFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FRecallAbilityAnimationFragment>(EMassFragmentAccess::ReadWrite);	
	EntityQuery.AddRequirement<FRecallGameplayTagFragment>(EMassFragmentAccess::ReadWrite, EMassFragmentPresence::Optional);
	EntityQuery.RegisterWithProcessor(*this);	
}

void URecallAbilityProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	QUICK_SCOPE_CYCLE_COUNTER(Recall_AbilityExecution_Process);

	// Batch process entities using chunk-based iteration for performance optimization
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		// Acquire mutable fragment views for ability state management
		const TArrayView<FRecallAbilityFragment> AbilityFragments = Context.GetMutableFragmentView<FRecallAbilityFragment>();
		const TArrayView<FRecallAbilityAnimationFragment> AnimationFragments = Context.GetMutableFragmentView<FRecallAbilityAnimationFragment>();
		const TArrayView<FRecallGameplayTagFragment> TagFragments = Context.GetMutableFragmentView<FRecallGameplayTagFragment>();
		
		// Process each entity within the chunk using optimized batch operations
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); EntityIndex++)
		{
			const FMassEntityHandle Entity = Context.GetEntity(EntityIndex);
			
			FRecallAbilityFragment& AbilityFragment = AbilityFragments[EntityIndex];
			FRecallAbilityAnimationFragment& AnimationFragment = AnimationFragments[EntityIndex];
			
			// Optional gameplay tag access for enhanced ability functionality
			FRecallGameplayTagFragment* const TagsPtr = TagFragments.IsValidIndex(EntityIndex) ? &TagFragments[EntityIndex] : nullptr;
			
			// Construct execution context for ability command processing
			const FRecallAbilityExecutionContext ExecutionContext{
				Context, Entity, AbilityFragment, TagsPtr
			};
			
			// Execute active ability operations when not in transition state
			if (AbilityFragment.CurrentAbility && !AbilityFragment.bTransitionTriggered)
			{
				// Process continuous ability execution
				Recall::Ability::Utils::ProcessAbilityCommandSequence(
					AbilityFragment.CurrentAbility, ExecutionContext, ERecallAbilityExecutionEvent::OnTick);

				// Handle animation sequence completion notifications
				if (AnimationFragment.bTimelineEnd)
				{
					Recall::Ability::Utils::ProcessAbilityCommandSequence(
						AbilityFragment.CurrentAbility, ExecutionContext, ERecallAbilityExecutionEvent::OnAnimEnd);
				}
			}

			// Manage ability lifecycle transitions and state cleanup
			if (AnimationFragment.bTimelineEnd && !AbilityFragment.bTransitionTriggered)
			{
				// Terminate current ability and prepare for idle state
				Recall::Ability::Utils::ExitCurrentAbility(ExecutionContext);
				AnimationFragment.bChangeAnimation = true;
			}
			else
			{				
				// Execute pending ability sequence transitions
				Recall::Ability::Utils::ProcessAbilitySequence(ExecutionContext, AnimationFragment);
			}
		}
	});
}

//----------------------------------------------------------------------//
// URecallAbilityRepresentationProcessor
//----------------------------------------------------------------------//
#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
static bool bDebugShowAbility = false;
static FAutoConsoleVariableRef CVarRecallShowAbility(
	TEXT("Recall.Ability.ShowAll"),
	bDebugShowAbility,
	TEXT("Show Ability")
);
#endif // UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT

URecallAbilityRepresentationProcessor::URecallAbilityRepresentationProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = static_cast<int32>(EExtendedProcessorExecutionFlags::All);
	ProcessingPhase = EMassProcessingPhase::Render;
	bRequiresGameThreadExecution = true;
}

void URecallAbilityRepresentationProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager)
{
	Super::InitializeInternal(Owner, InEntityManager);
}

void URecallAbilityRepresentationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	EntityQuery.AddRequirement<FRecallTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FRecallAbilityFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FRecallAbilityAnimationFragment>(EMassFragmentAccess::ReadOnly);	
#endif // UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
}

void URecallAbilityRepresentationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	QUICK_SCOPE_CYCLE_COUNTER(Recall_AbilityRepresentation_Execute);

	if (!bDebugShowAbility)
	{
		return;
	}

	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		const TConstArrayView<FRecallAbilityFragment> AbilityList = Context.GetFragmentView<FRecallAbilityFragment>();
		const TConstArrayView<FRecallAbilityAnimationFragment> AnimationList = Context.GetFragmentView<FRecallAbilityAnimationFragment>();
		const TConstArrayView<FRecallTransformFragment> TransformList = Context.GetFragmentView<FRecallTransformFragment>();
		
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); EntityIndex++)
		{
			const FRecallAbilityFragment& AbilityFragment = AbilityList[EntityIndex];
			
			const FRecallAbilityAnimationFragment& AnimationFragment = AnimationList[EntityIndex];
			const FRecallTransformFragment& TransformFragment = TransformList[EntityIndex];

			FString DebugString;
			
			if (AbilityFragment.CurrentAbility)
			{
				const TArray<FRecallAbilityAnimationSection>& AnimationSections = AbilityFragment.CurrentAbility->AnimationSections;
				const FName AnimationSection = AnimationSections.IsValidIndex(AnimationFragment.CurrentSectionIndex) ?
					AnimationSections[AnimationFragment.CurrentSectionIndex].Name : NAME_None;
				
				DebugString += FString::Printf(TEXT("%s: %d / %d\n"), *AbilityFragment.CurrentAbility->GetName(),
					AbilityFragment.CurrentFrame, AbilityFragment.CurrentAbility->GetDuration());
				DebugString += FString::Printf(TEXT("* Section: %s\n"), *AnimationSection.ToString());
				DebugString += FString::Printf(TEXT("* Time: %d\n"), AbilityFragment.ElapsedFrames);
			}
			else
			{
				DebugString += TEXT("none");
			}
			
			DrawDebugString(Context.GetWorld(), TransformFragment.Position, DebugString,
				nullptr, FColor::White, 0.f, true);
		}
	});
#endif // UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
}
