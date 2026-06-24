// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallAbilityAnimationProcessors.h"

#include "Animation/RecallAbilityAnimInstance.h"
#include "Animation/SkeletalMeshActor.h"
#include "Data/Ability/RecallAbilityAsset.h"
#include "MassExecutionContext.h"
#include "RecallSignalSubsystem.h"
#include "Actor/RecallSkeletalMeshActorRepresentationInterface.h"
#include "Simulation/Ability/RecallAbilityFragments.h"
#include "Simulation/Ability/RecallAbilityProcessorGroupTypes.h"
#include "Simulation/Representation/RecallActorRepresentationFragments.h"
#include "Simulation/StateTree/RecallStateTreeSignalTypes.h"
#include "System/Actor/RecallActorSubsystem.h"
#include "System/Slowmo/RecallSlowMotionSubsystem.h"
#include "Utility/Simulation/RecallSimulationUtils.h"

//----------------------------------------------------------------------//
// URecallAbilityAnimationProcessor
//----------------------------------------------------------------------//
URecallAbilityAnimationProcessor::URecallAbilityAnimationProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = static_cast<int32>(EExtendedProcessorExecutionFlags::All);
	ProcessingPhase = EMassProcessingPhase::StartPhysics;
	ExecutionOrder.ExecuteInGroup = Recall::Ability::ProcessorGroupNames::StartPhysics::Animation;
}

void URecallAbilityAnimationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FRecallAbilityAnimationFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FRecallAbilityFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddSubsystemRequirement<URecallSlowMotionSubsystem>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSubsystemRequirement<URecallSignalSubsystem>(EMassFragmentAccess::ReadWrite);
}

// Animation progression state evaluator using threshold-based progression analysis
static bool ValidateSegmentProgression(const FRecallAbilityFragment& AbilityFragment, const FRecallAbilityAnimationFragment& AnimationFragment)
{
	// Validate ability context and segment bounds
	if (!ensure(AbilityFragment.CurrentAbility))
	{
		return false;
	}

	const TArray<FRecallAbilityAnimationSection>& Segments = AbilityFragment.CurrentAbility->AnimationSections;
	if (!Segments.IsValidIndex(AnimationFragment.CurrentSectionIndex))
	{
		return false;
	}

	// Calculate progression using segment offset methodology
	const FRecallAbilityAnimationSection& ActiveSegment = Segments[AnimationFragment.CurrentSectionIndex];
	const int32 SegmentStartPosition = AbilityFragment.CurrentAbility->GetAnimationSectionStartFrame(AnimationFragment.CurrentSectionIndex);
	const int32 SegmentEndPosition = SegmentStartPosition + ActiveSegment.Duration;

	// Threshold-based completion detection
	return AbilityFragment.CurrentFrame >= SegmentEndPosition;
}

// Continuous execution evaluator using iteration-based analysis
static bool CheckContinuousExecution(const FRecallAbilityFragment& AbilityFragment, const FRecallAbilityAnimationFragment& AnimationFragment)
{
	// Validate segment access permissions
	if (!ensure(AbilityFragment.CurrentAbility))
	{
		return false;
	}

	const TArray<FRecallAbilityAnimationSection>& Segments = AbilityFragment.CurrentAbility->AnimationSections;
	if (!Segments.IsValidIndex(AnimationFragment.CurrentSectionIndex))
	{
		return false;
	}

	// Evaluate repetition criteria using reverse logic pattern
	const FRecallAbilityAnimationSection& ActiveSegment = Segments[AnimationFragment.CurrentSectionIndex];
	if (!ActiveSegment.bLoop)
	{
		return false;
	}

	// Unlimited repetition path
	if (ActiveSegment.LoopCount == 0)
	{
		return true;
	}

	// Bounded repetition analysis
	const int32 NextIterationCount = AnimationFragment.CurrentSectionLoopCount + 1;
	return NextIterationCount < ActiveSegment.LoopCount;
}

// Sequential segment resolver using navigation chain analysis  
static int32 ResolveNextSequentialSegment(const FRecallAbilityFragment& AbilityFragment, const FRecallAbilityAnimationFragment& AnimationFragment)
{
	// Verify segment collection integrity
	const TArray<FRecallAbilityAnimationSection>& SegmentCollection = AbilityFragment.CurrentAbility->AnimationSections;
	if (!SegmentCollection.IsValidIndex(AnimationFragment.CurrentSectionIndex))
	{
		return INDEX_NONE;
	}

	const FRecallAbilityAnimationSection& CurrentSegment = SegmentCollection[AnimationFragment.CurrentSectionIndex];
	
	// Named segment navigation pathway
	if (!CurrentSegment.NextSection.IsNone())
	{
		// Search segment collection for named target
		for (int32 SegmentIdx = 0; SegmentIdx < SegmentCollection.Num(); ++SegmentIdx)
		{
			if (SegmentCollection[SegmentIdx].Name == CurrentSegment.NextSection)
			{
				return SegmentIdx;
			}
		}
		return INDEX_NONE;
	}

	// Sequential progression pathway
	const int32 NextSequentialIndex = AnimationFragment.CurrentSectionIndex + 1;
	const int32 MaxSegmentIndex = SegmentCollection.Num() - 1;
	
	// Boundary validation for sequential advancement
	if (NextSequentialIndex <= MaxSegmentIndex)
	{
		return NextSequentialIndex;
	}

	return INDEX_NONE;
}

void URecallAbilityAnimationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	QUICK_SCOPE_CYCLE_COUNTER(Recall_AbilityAnimation_Execute);

	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		const URecallSlowMotionSubsystem& SlowMotionSystem = Context.GetSubsystemChecked<URecallSlowMotionSubsystem>();
		URecallSignalSubsystem& SignalSystem = Context.GetMutableSubsystemChecked<URecallSignalSubsystem>();
		
		const float TimeDilatation = SlowMotionSystem.GetTimeDilatation();

		const TArrayView<FRecallAbilityFragment> AbilityList = Context.GetMutableFragmentView<FRecallAbilityFragment>();
		const TArrayView<FRecallAbilityAnimationFragment> AnimationList = Context.GetMutableFragmentView<FRecallAbilityAnimationFragment>();

		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); EntityIndex++)
		{
			const FMassEntityHandle Entity = Context.GetEntity(EntityIndex);
			FRecallAbilityFragment& AbilityFragment = AbilityList[EntityIndex];
			FRecallAbilityAnimationFragment& AnimationFragment = AnimationList[EntityIndex];

			AnimationFragment.bTimelineEnd = false;
			AbilityFragment.bCurrentFrameUpdated = false;

			if (!AbilityFragment.CurrentAbility)
			{
				continue;
			}

			// Advance temporal tracking for active ability execution.
			AbilityFragment.ElapsedFrames++;
			AbilityFragment.ElapsedDilatedFrames += static_cast<double>(TimeDilatation);

			const TObjectPtr<UObject> LastAnimation = AnimationFragment.AnimationAsset;
							
			// Execute frame-based progression using event-triggered workflow
			if (AbilityFragment.IncrementCurrentFrame(TimeDilatation))
			{
				// Initialize animation processing state for this execution cycle
				bool bProgressionEventTriggered = false;
				bool bSegmentTransitionRequired = false;
				int32 TargetSegmentIndex = INDEX_NONE;

				// Evaluate segment progression through event-based analysis
				if (AnimationFragment.CurrentSectionIndex < AbilityFragment.CurrentAbility->AnimationSections.Num())
				{
					// Trigger progression validation event
					if (ValidateSegmentProgression(AbilityFragment, AnimationFragment))
					{
						bProgressionEventTriggered = true;

						if (CheckContinuousExecution(AbilityFragment, AnimationFragment))
						{
							// Handle repetition event through timeline reset protocol
							const int32 SegmentOriginFrame = AbilityFragment.CurrentAbility->GetAnimationSectionStartFrame(AnimationFragment.CurrentSectionIndex);
							AbilityFragment.SetCurrentFrame(SegmentOriginFrame);

							const FRecallAbilityAnimationSection& ExecutingSegment = AbilityFragment.CurrentAbility->AnimationSections[AnimationFragment.CurrentSectionIndex];
							if (ExecutingSegment.LoopCount > 0)
							{
								AnimationFragment.CurrentSectionLoopCount++;
							}
						}
						else
						{
							// Prepare segment transition event
							TargetSegmentIndex = ResolveNextSequentialSegment(AbilityFragment, AnimationFragment);
							bSegmentTransitionRequired = true;
						}
					}
				}
				// Handle ability completion through termination event trigger
				else if (AbilityFragment.CurrentFrame >= AbilityFragment.CurrentAbility->GetDuration())
				{
					bProgressionEventTriggered = true;
					AnimationFragment.bTimelineEnd = true;
				}

				// Process segment transition event when triggered
				if (bSegmentTransitionRequired)
				{
					if (TargetSegmentIndex != INDEX_NONE)
					{
						AnimationFragment.CurrentSectionLoopCount = 0;
						AnimationFragment.CurrentSectionIndex = TargetSegmentIndex;
						AnimationFragment.bTimelineEnd = false;
					}
					else
					{
						AnimationFragment.bTimelineEnd = true;
					}
				}

				// Execute asset update event when progression occurred
				if (bProgressionEventTriggered && AnimationFragment.CurrentSectionIndex < AbilityFragment.CurrentAbility->AnimationSections.Num())
				{
					const FRecallAbilityAnimationSection& TargetAnimationSegment = AbilityFragment.CurrentAbility->AnimationSections[AnimationFragment.CurrentSectionIndex];
					const TObjectPtr<UObject> UpdatedAnimation = TargetAnimationSegment.GetAnimationAsset();
					if (UpdatedAnimation && UpdatedAnimation != LastAnimation)
					{
						AnimationFragment.AnimationAsset = UpdatedAnimation;
					}
				}

				// Publish animation modification event to rendering pipeline
				AnimationFragment.bChangeAnimation = AnimationFragment.AnimationAsset != LastAnimation;

				// Dispatch completion event to state management subsystem
				if (AnimationFragment.bTimelineEnd)
				{
					SignalSystem.SignalEntity(Recall::StateTree::Signals::TickRequired, Entity);
				}
			}
		}
	});
}

//----------------------------------------------------------------------//
// URecallAbilityAnimationSynchronizationProcessor
//----------------------------------------------------------------------//
URecallAbilityAnimationSynchronizationProcessor::URecallAbilityAnimationSynchronizationProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = static_cast<int32>(EExtendedProcessorExecutionFlags::All);
	ProcessingPhase = EMassProcessingPhase::EndPhysics;
	ExecutionOrder.ExecuteInGroup = Recall::Ability::ProcessorGroupNames::EndPhysics::AnimationSync;
}

void URecallAbilityAnimationSynchronizationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FRecallAbilityAnimationFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FRecallAbilityFragment>(EMassFragmentAccess::ReadOnly);
}

void URecallAbilityAnimationSynchronizationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	QUICK_SCOPE_CYCLE_COUNTER(Recall_AbilityAnimation_Synchronization);

	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
	{
		const TConstArrayView<FRecallAbilityFragment> AbilityList = Context.GetFragmentView<FRecallAbilityFragment>();
		const TArrayView<FRecallAbilityAnimationFragment> AnimationList = Context.GetMutableFragmentView<FRecallAbilityAnimationFragment>();

		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); EntityIndex++)
		{
			const FRecallAbilityFragment& AbilityFragment = AbilityList[EntityIndex];
			FRecallAbilityAnimationFragment& AnimationFragment = AnimationList[EntityIndex];

			// Signal rendering system when animation assets require montage activation.
			AnimationFragment.bPlayNewAnimationMontage = AnimationFragment.bChangeAnimation;
			
			// Configure transition smoothing parameters for asset changes.
			if (AnimationFragment.bChangeAnimation)
			{
				if (AbilityFragment.CurrentAbility)
				{
					AnimationFragment.bChangeAnimation = false;
					AnimationFragment.BlendTimer = 0.0f;			

					// Establish transition parameters for ability state progression.
					if(AbilityFragment.CurrentAbility->BlendIn > 0)
					{
						AnimationFragment.BlendDuration = AbilityFragment.CurrentAbility->BlendIn;

						// Offset initial blend timing to avoid pose duplication artifacts.
						AnimationFragment.BlendTimer = 1.f;
					}
					// Disable blending for abilities without transition timing configuration.
					else
					{
						AnimationFragment.BlendDuration = 0;
					}

					// Skip blending for intra-ability animation changes like segment transitions.
					if (AbilityFragment.ElapsedFrames > 0)
					{
						AnimationFragment.BlendDuration = 0.f;
					}
				}
				else
				{
					AnimationFragment.AnimationAsset = nullptr;
					AnimationFragment.BlendDuration = 0.f;
				}
			} 
			
			// Bypass blending when orientation changes require immediate visual updates.
			if (AnimationFragment.bForceResetBlend)
			{
				AnimationFragment.BlendDuration = 0;
			}
			
			// Calculate blend interpolation factor based on elapsed transition time
			// since previous ability completion.
			float BlendAmount = 1.f;
			if (AnimationFragment.BlendDuration > 0)
			{
				BlendAmount = AnimationFragment.BlendTimer / AnimationFragment.BlendDuration;

				if (BlendAmount >= 1.0f)
				{
					BlendAmount = 1.0f;
				}
			}

			// Synchronize blend values with animation system parameters
			AnimationFragment.AnimInstanceBlend = BlendAmount;
			AnimationFragment.AnimInstanceLocomotionBlend = AbilityFragment.CurrentAbility ? 0.f : 1.0f;

			// Advance blend progression timing
			if (AnimationFragment.BlendTimer < AnimationFragment.BlendDuration)
			{
				const float SimulationRate = 1.f;
				// Update blend timing using simulation time delta
				if (SimulationRate > 0)
				{
					AnimationFragment.BlendTimer += SimulationRate * 1.f;
				}

				if (AnimationFragment.BlendTimer > AnimationFragment.BlendDuration)
				{
					AnimationFragment.BlendTimer = AnimationFragment.BlendDuration;
				}
			}
		}
	});
}

//----------------------------------------------------------------------//
// URecallAbilityAnimationSkeletalMeshRepresentationProcessor
//----------------------------------------------------------------------//
URecallAbilityAnimationSkeletalMeshRepresentationProcessor::URecallAbilityAnimationSkeletalMeshRepresentationProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = static_cast<int32>(EExtendedProcessorExecutionFlags::All);
	ProcessingPhase = EMassProcessingPhase::Render;
	bRequiresGameThreadExecution = true;
}

void URecallAbilityAnimationSkeletalMeshRepresentationProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager)
{
	Super::InitializeInternal(Owner, InEntityManager);
}

void URecallAbilityAnimationSkeletalMeshRepresentationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FRecallAbilityAnimationFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FRecallActorRepresentationFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FRecallAbilityFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddTagRequirement<FRecallSkeletalMeshActorRepresentationTag>(EMassFragmentPresence::All);
	EntityQuery.AddSubsystemRequirement<URecallActorSubsystem>(EMassFragmentAccess::ReadOnly);
}

static USkeletalMeshComponent* GetSkeletalMeshComponent(const TWeakObjectPtr<AActor>& Actor)
{
	if (const ASkeletalMeshActor* SkeletalMeshActor = Cast<ASkeletalMeshActor>(Actor.Get()))
	{
		return SkeletalMeshActor->GetSkeletalMeshComponent();
	}
	else
	{
		return IRecallSkeletalMeshActorRepresentationInterface::Execute_GetSkeletalMeshComponent(Actor.Get());
	}
}

void URecallAbilityAnimationSkeletalMeshRepresentationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	QUICK_SCOPE_CYCLE_COUNTER(Recall_AbilityAnimationSkeletalMesh_Representation);

	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		const uint32 Frame = Recall::Simulation::Utils::GetFrame(Context.GetWorld());		
		const float FixedDeltaTime = Recall::Simulation::Utils::GetFixedDeltaTime(Context.GetWorld());

		const URecallActorSubsystem& ActorSystem = Context.GetSubsystemChecked<URecallActorSubsystem>();
		
		const TConstArrayView<FRecallAbilityFragment> AbilityList = Context.GetFragmentView<FRecallAbilityFragment>();
			
		const TConstArrayView<FRecallActorRepresentationFragment> ActorList = Context.GetFragmentView<FRecallActorRepresentationFragment>();
		const TConstArrayView<FRecallAbilityAnimationFragment> AnimationList = Context.GetFragmentView<FRecallAbilityAnimationFragment>();
		
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); EntityIndex++)
		{
			const FRecallAbilityFragment& AbilityFragment = AbilityList[EntityIndex];			
			const FRecallActorRepresentationFragment& ActorFragment = ActorList[EntityIndex];
			const FRecallAbilityAnimationFragment& AnimationFragment = AnimationList[EntityIndex];
			
			const TWeakObjectPtr<AActor> Actor = ActorSystem.GetActor(ActorFragment.ActorHandle);
			if (!Actor.IsValid())
			{
				continue;
			}

			Actor->ForEachComponent<USkeletalMeshComponent>(false, [&](USkeletalMeshComponent* Component)
			{				
				URecallAbilityAnimInstance* AnimInstance = Cast<URecallAbilityAnimInstance>(Component->GetAnimInstance());
				if (IsValid(AnimInstance))
				{
					// Limit montage updates to single occurrence per simulation frame. 
					bool bPlayNewAnimationMontage = false;
					const int32 LastGameFrame = Frame;
					const bool bFirstTickOfAnimInst = AnimInstance->LastMontageChangeGameFrame == 0;
						
					if (AnimationFragment.bPlayNewAnimationMontage && LastGameFrame > AnimInstance->LastMontageChangeGameFrame)
					{
						bPlayNewAnimationMontage = true;
						AnimInstance->LastMontageChangeGameFrame = LastGameFrame;
					}

					// Process animation asset activation
					if (bPlayNewAnimationMontage)
					{						
						if (UAnimationAsset* AnimationAsset = Cast<UAnimationAsset>(AnimationFragment.AnimationAsset))
						{
							// Handle multi-directional animation assets with blendspace playback
							if (UBlendSpace* BlendSpace = Cast<UBlendSpace>(AnimationAsset))
							{
								AnimInstance->PlayBlendSpace(BlendSpace);
							}
							else
							{
		#if WITH_EDITOR
								if (!AnimInstance->CurrentSkeleton->IsCompatibleForEditor(AnimationAsset->GetSkeleton()))
								{
									UE_LOG(LogRecallActor, Warning,
										TEXT("Incompatible skeleton for the editor, AnimInstance's skeleton is %s but AnimationAsset's skeleton is %s"),
										*AnimInstance->CurrentSkeleton->GetFullName(), *AnimationAsset->GetSkeleton()->GetFullName());
								}
		#endif // WITH_EDITOR

								AnimInstance->PlayAnimationAsMontage(Cast<UAnimSequenceBase>(AnimationAsset));
							}
						}
					}
					else if (!AnimationFragment.AnimationAsset)
					{
						AnimInstance->ClearAnimation();
					}

					{
						// Align animation playback position with ability execution timeline.
						AnimInstance->SynchronizeToAbilityFrame(AbilityFragment.CurrentAbility, AbilityFragment.CurrentFrameTime);
					}

					// Update animation system blend parameters
					AnimInstance->AnimBlend = AnimationFragment.AnimInstanceBlend;
					AnimInstance->LocomotionBlend = AnimationFragment.AnimInstanceLocomotionBlend;

					// const bool StopAnimTick = SimulationRate <= 0 || AnimationFragment.bPauseAnimation;

					// We disable the tick only when the delta time would be 0 and there's no animation to change.
					// If we don't disable the tick the Unreal animation system will call any notify state's Begin events multiple times.
					// SkeletalMesh->EnableExternalUpdate(bPlayNewAnimationMontage || !StopAnimTick);

					// Handle initial frame pose update for newly visible actors.
					if (bFirstTickOfAnimInst)
					{
						Component->TickPose(FixedDeltaTime, false);
					}
				}
				else
				{
					Component->SetExternalDeltaTime(FixedDeltaTime);
					Component->EnableExternalUpdate(true);
					Component->EnableExternalTickRateControl(true);
				}
			});
		}
	});
}