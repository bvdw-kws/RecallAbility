// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "Animation/AnimInstance.h"
#include "Animation/RecallAnimInstance.h"

#include "RecallAbilityAnimInstance.generated.h"

/**
 * Recall Ability Animation Instance
 * 
 * Specialized animation controller for Recall ability system providing:
 * - Deterministic ability animation synchronization for multiplayer rollback
 * - Ability-centric animation section management with frame-perfect timing
 * - Recall ECS integration with external tick control
 * - Support for both blend spaces and montage-based ability animations
 * - Smooth transitions between ability states via pose snapshots
 */

UCLASS(Blueprintable, BlueprintType, Within=SkeletalMeshComponent, MinimalAPI)
class URecallAbilityAnimInstance : public URecallAnimInstance
{
	GENERATED_BODY()

public:
	/**
	 * Activates a blend space animation for the current ability execution.
	 * Configures blend space parameters and captures previous pose for smooth transitions.
	 * @param BlendSpace The blend space asset to activate for ability animation
	 */
	UFUNCTION(BlueprintCallable)
	void PlayBlendSpace(UBlendSpace* BlendSpace);
	
	/**
	 * Executes an animation sequence as a dynamic montage for ability playback.
	 * Creates a runtime montage with ability-specific settings for deterministic playback.
	 * @param Animation The animation sequence to execute
	 * @param bLoop Whether the animation should loop continuously
	 * @return The created dynamic montage, or nullptr if creation failed
	 */
	UFUNCTION(BlueprintCallable)
	UAnimMontage* PlayAnimationAsMontage(UAnimSequenceBase* Animation, bool bLoop = false);

	/**
	 * Immediately terminates the current ability animation playback.
	 * Used for ability interruption or forced transitions.
	 */
	UFUNCTION(BlueprintCallable)
	void ClearAnimation();
	
	/**
	 * Synchronizes ability animation to a specific frame for deterministic playback.
	 * Core method for Recall rollback netcode integration, ensuring frame-perfect
	 * animation synchronization across all clients.
	 * @param Ability The ability asset defining animation sections and timing
	 * @param Frame The target frame to synchronize to within the ability timeline
	 */
	UFUNCTION(BlueprintCallable)
	void SynchronizeToAbilityFrame(const URecallAbilityAsset* Ability, float Frame);

public:
	/** Frame tracking for ability animation state changes (Recall rollback synchronization) */
	UPROPERTY(Transient)
	int32 LastMontageChangeGameFrame = 0;

	/** General animation blend factor for ability transitions */
	UPROPERTY(Transient, BlueprintReadOnly, Category=Animation)
	float AnimBlend = 0.0f;
	
	/** X-axis input parameter for ability blend space evaluation */
	UPROPERTY(Transient, BlueprintReadOnly, Category="Ability Animation")
	float BlendSpaceInputX = 0.0f;
	
	/** Y-axis input parameter for ability blend space evaluation */
	UPROPERTY(Transient, BlueprintReadOnly, Category="Ability Animation")
	float BlendSpaceInputY = 0.0f;
	
	/** Blend factor between ability animations and movement system */
	UPROPERTY(Transient, BlueprintReadOnly, Category=Animation)
	float LocomotionBlend = 1.0f;
	
protected:
	/** Currently active blend space asset for ability animation */	
	UPROPERTY(BlueprintReadOnly, Transient, Category=Animation)
	TObjectPtr<UBlendSpace> AbilityBlendSpace;
	
	/** Activation state for ability blend space (0=inactive, 1=fully active) */
	UPROPERTY(BlueprintReadOnly, Transient, Category=Animation)
	float BlendSpaceRate = 0.f;

	/** Playback rate multiplier for current ability animation section */
	UPROPERTY(BlueprintReadOnly, Transient, Category=Animation)
	float BlendSpacePlayRate = 1.f;

	/** Frame offset into the current ability animation section */
	UPROPERTY(BlueprintReadOnly, Transient, Category=Animation)
	float BlendSpaceStartOffset = 0.f;

	/** Whether the current ability animation section should loop */
	UPROPERTY(BlueprintReadOnly, Transient, Category=Animation)
	bool bShouldBlendSpaceLoop = false;

};
