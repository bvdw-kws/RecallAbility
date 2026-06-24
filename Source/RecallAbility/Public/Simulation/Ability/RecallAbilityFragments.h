// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "MassEntityElementTypes.h"
#include "MassEntityHandle.h"

#include "RecallAbilityFragments.generated.h"

class URecallAbilityAsset;
class URecallAbilityCollectionAsset;

USTRUCT()
struct RECALLABILITY_API FRecallAbilityFragment : public FMassFragment
{
	GENERATED_BODY()

	/**
	 * Repository of available abilities accessible through gameplay tag system.
	 */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<const URecallAbilityCollectionAsset> AbilityCollection;

	/**
	 * The active ability currently executing on this entity.
	 */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<const URecallAbilityAsset> CurrentAbility;

	/**
	 * Queued ability scheduled for activation upon current ability completion.
	 */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<const URecallAbilityAsset> NextAbility;
	
	/**
	 * Designated animation segment for the pending ability activation.
	 */
	UPROPERTY(VisibleAnywhere)
	FName NextSection = NAME_None;

	/**
	 * Flag indicating a pending ability transition request.
	 */
	UPROPERTY(VisibleAnywhere)
	bool bTransitionTriggered = false;

	/**
	 * Bypass blending calculations for immediate ability activation.
	 */
	UPROPERTY(VisibleAnywhere)
	bool bIgnoreBlend = false;

	/**
	 * Collection of entity references within ability targeting scope.
	 */
	UPROPERTY(VisibleAnywhere)
	TArray<FMassEntityHandle> TargetEntities;

	/**
	 * Current discrete time unit within the active animation segment.
	 */
	UPROPERTY(VisibleAnywhere)
	int32 CurrentFrame = 0;

	/**
	 * Precise timing position within the current animation sequence.
	 */
	UPROPERTY(VisibleAnywhere)
	float CurrentFrameTime = 0;

	/**
	 * Total frame count since ability activation commenced.
	 */
	UPROPERTY(VisibleAnywhere)
	int32 ElapsedFrames = 0;

	/**
	 * Accumulated time including temporal scaling effects since ability start.
	 */
	UPROPERTY(VisibleAnywhere)
	double ElapsedDilatedFrames = 0.0;

	/**
	 * Speed multiplier governing ability animation playback velocity.
	 */
	UPROPERTY(VisibleAnywhere)
	float ActionPlayRate = 1.f;
	
	/**
	 * Detection flag for frame progression within the current update cycle.
	 */
	UPROPERTY(VisibleAnywhere)
	bool bCurrentFrameUpdated = false;
	
	FORCEINLINE void SetCurrentFrame(int32 Frame)
	{
		const int32 OldFrame = static_cast<int32>(CurrentFrameTime);

		CurrentFrameTime = Frame;
		CurrentFrame = Frame;
		
		bCurrentFrameUpdated = CurrentFrame != OldFrame;
	}
	
	FORCEINLINE bool IncrementCurrentFrame(float TimeDilatation = 1.0f)
	{
		const int32 OldFrame = CurrentFrame;
		
		CurrentFrameTime += ActionPlayRate * TimeDilatation;
		CurrentFrame = static_cast<int32>(CurrentFrameTime);
		
		bCurrentFrameUpdated = CurrentFrame > OldFrame;
		
		return bCurrentFrameUpdated;
	}
	
	bool TransitionToAbility(const URecallAbilityAsset* Ability,
		bool bNewIgnoreBlend = false, FName Section = NAME_None,
		TOptional<TArray<FMassEntityHandle>> Targets = TOptional<TArray<FMassEntityHandle>>());
	
	FORCEINLINE void ResetAbilityFrame()
	{
		SetCurrentFrame(0);
		ActionPlayRate = 1.f;
		ElapsedFrames = 0;
		ElapsedDilatedFrames = 0.0;
		bCurrentFrameUpdated = true;
	}
};

USTRUCT()
struct RECALLABILITY_API FRecallAbilityAnimationFragment : public FMassFragment
{
	GENERATED_BODY()

	/**
	 * Cached reference to the animation asset for the active ability.
	 */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UObject> AnimationAsset;
	
	/**
	 * Index of the currently playing animation segment.
	 */
	UPROPERTY(VisibleAnywhere)
	int32 CurrentSectionIndex = 0;

	UPROPERTY(VisibleAnywhere)
	bool bForceResetBlend = false;
	
	UPROPERTY(VisibleAnywhere)
	float BlendDuration = 0.f;
	
	UPROPERTY(VisibleAnywhere)
	float BlendTimer = 0.f;
	
	UPROPERTY(VisibleAnywhere)
	float AnimInstanceBlend = 1.f;

	/**
	 * Weight factor for blending between movement and ability animations.
	 */
	UPROPERTY(VisibleAnywhere)
	float AnimInstanceLocomotionBlend = 0.0f;
	
	/**
	 * Notification flag for rendering system when animation assets change.
	 */
	UPROPERTY(VisibleAnywhere)
	bool bChangeAnimation = false;
	
	/**
	 * Signal for rendering system to initiate new animation montage playback.
	 */	
	UPROPERTY(VisibleAnywhere)
	bool bPlayNewAnimationMontage = false;

	/**
	 * Count of repetitions completed for the active animation segment.
	 */
	UPROPERTY(VisibleAnywhere)
	int32 CurrentSectionLoopCount = 0;
	
	/**
	 * Flag marking completion of the current ability's execution sequence.
	 */
	UPROPERTY(VisibleAnywhere)
	bool bTimelineEnd = false;

	/**
	 * Historical flag tracking the previous ability's completion state.
	 */
	UPROPERTY(VisibleAnywhere)
	bool bLastTimelimeEnded = false;
	
	FORCEINLINE void ResetOnEnterAbility()
	{
		CurrentSectionIndex = 0;
		CurrentSectionLoopCount = 0;
		bLastTimelimeEnded = bTimelineEnd;
		bTimelineEnd = false;
	}
};
