// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallAbilityAnimInstance.h"

#include "Animation/AnimationSettings.h"
#include "Data/Ability/RecallAbilityAsset.h"
#include "Utility/Simulation/RecallSimulationUtils.h"

void URecallAbilityAnimInstance::PlayBlendSpace(UBlendSpace* BlendSpace)
{
	// Capture current pose for smooth transition from previous ability state.
	// This ensures visual continuity when switching between different ability animations.
	SavePoseSnapshot(TEXT("PreviousPose"));
	
	// Configure animation graph to use ability blend space instead of locomotion.
	// This switches the character from movement-based to ability-based animation.
	BlendSpaceRate = 1.f;  // Fully activate ability blend space
	LocomotionBlend = 0.f;          // Disable movement system blending
	AbilityBlendSpace = BlendSpace; // Store reference for animation graph
}

UAnimMontage* URecallAbilityAnimInstance::PlayAnimationAsMontage(UAnimSequenceBase* Animation, bool bLoop /*= false*/)
{
	// Disable movement system to ensure ability animation takes full control.
	LocomotionBlend = 0.f;
	
	// Create dynamic montage for ability animation playback.
	// Uses DefaultSlot for ability animations to ensure proper layering.
	UAnimMontage* NewMontage = UAnimMontage::CreateSlotAnimationAsDynamicMontage(
		Animation, TEXT("DefaultSlot"), 0, 0, 0.f, 1);

	// Capture pose for smooth transition into ability animation.
	SavePoseSnapshot(TEXT("PreviousPose"));

	if (NewMontage)
	{
		// Disable automatic blend out to maintain ability control over animation timing.
		// This ensures abilities can control their own exit transitions.
		NewMontage->bEnableAutoBlendOut = false;

		// Start montage playback at standard rate for deterministic timing.
		Montage_Play(NewMontage, 1.f, EMontagePlayReturnType::MontageLength);

		// Configure looping behavior for abilities that require continuous animation.
		if (bLoop)
		{
			Montage_SetNextSection(TEXT("Default"), TEXT("Default"), NewMontage);
		}
	}
	else
	{
		// Log warning for debugging ability animation setup issues.
		UE_LOG(LogTemp, Warning, TEXT("Failed to create ability animation montage"));
	}

	return NewMontage;
}

void URecallAbilityAnimInstance::ClearAnimation()
{
	// Immediately stop current ability animation for instant transitions or interruptions.
	// Zero blend-out time ensures immediate termination for responsive ability system.
	Montage_Stop(0.0f);
}

void URecallAbilityAnimInstance::SynchronizeToAbilityFrame(const URecallAbilityAsset* Ability, float Frame)
{
	// Validate ability asset before attempting synchronization.
	if (!Ability)
	{
		return;
	}
	
	// Calculate fixed delta time for deterministic frame-based animation timing.
	// This ensures consistent timing across all clients in multiplayer scenarios.
	const float FixedDeltaTime = 1.0f / static_cast<float>(UAnimationSettings::Get()->GetDefaultFrameRate().Numerator);
	
	// Initialize animation offset for frame synchronization.
	float AnimationOffset = 0.f;

	// Process ability animation sections for frame-accurate playback.
	if (Ability->AnimationSections.Num() > 0)
	{
		// Find the active animation section for the current frame.
		const FRecallAbilityAnimationSection* ActiveSection = nullptr;
		const float SectionStartFrame = Ability->GetSectionStartFrame(Frame, ActiveSection);
		
		// Validate that we found a valid section for the current frame.
		if (!ActiveSection)
		{
			return; // No valid section found, cannot synchronize
		}
		
		// Extract animation frame range from the active section.
		const float AnimStartFrame = ActiveSection->AnimFrameRange.GetLowerBoundValue();
		const float AnimEndFrame = ActiveSection->AnimFrameRange.GetUpperBoundValue();

		// Calculate current position within the animation sequence.
		float CurrentAnimFrame = 0.0f;
		
		// Calculate precise animation timing based on ability section progress.
		// This ensures frame-perfect synchronization for multiplayer rollback.
		if (ActiveSection->Duration > 0)
		{
			const float SectionElapsed = Frame - SectionStartFrame;
			
			// Calculate normalized progress through this ability section (0.0 to 1.0).
			const float SectionProgress = SectionElapsed / static_cast<float>(ActiveSection->Duration);
			const float SectionAnimDuration = AnimEndFrame - AnimStartFrame;
			
			// Map section progress to specific animation frame.
			CurrentAnimFrame = static_cast<float>(AnimStartFrame) + SectionProgress * SectionAnimDuration;
		}

		// Execute the appropriate animation type for this ability section.
		// Supports both blend spaces and animation sequences for maximum flexibility.
		if (const TObjectPtr<UObject> AnimationAsset = ActiveSection->GetAnimationAsset())
		{
			// Handle blend space-based ability animations.
			if (UBlendSpace* BlendSpace = Cast<UBlendSpace>(AnimationAsset))
			{
				// Activate blend space for this ability section.
				PlayBlendSpace(BlendSpace);
				
				// Configure blend space playback parameters for deterministic timing.
				BlendSpaceStartOffset = static_cast<float>(AnimStartFrame);
				BlendSpacePlayRate = static_cast<float>(AnimEndFrame - AnimStartFrame);
				bShouldBlendSpaceLoop = ActiveSection->bLoop;
				
				// Calculate precise frame offset for blend space synchronization.
				AnimationOffset = CurrentAnimFrame * FixedDeltaTime;
			}
			// Handle animation sequence-based ability animations.
			else if (UAnimSequenceBase* AnimSequence = Cast<UAnimSequenceBase>(AnimationAsset))
			{
				// Execute animation sequence as dynamic montage for ability playback.
				PlayAnimationAsMontage(AnimSequence);
				
				// Calculate timing based on animation's native frame rate.
				const float AnimFrameRate = static_cast<float>(AnimSequence->GetSamplingFrameRate().Numerator);
				if (AnimFrameRate > 0.0f)
				{
					// Convert ability frame to animation time offset for montage synchronization.
					AnimationOffset = CurrentAnimFrame * (1.0f / AnimFrameRate);
				}
			}
		}
	}
	else
	{
		// Fallback timing calculation for abilities without animation sections.
		// Uses simple frame-based timing for basic ability synchronization.
		AnimationOffset = FixedDeltaTime * Frame;
	}
				
	// Apply calculated animation offset for deterministic ability synchronization.
	// This is critical for Recall rollback netcode to maintain frame-perfect sync.
	if (FAnimMontageInstance* MontageInstance = GetActiveMontageInstance())
	{
		// Set animation position to exact frame for deterministic playback.
		// Position accounts for next frame advancement to maintain sync accuracy.
		MontageInstance->SetPosition(AnimationOffset);
	}

	// Configure Recall external tick control for deterministic animation updates.
	// This ensures animation timing is controlled by the ECS system rather than engine tick.
	USkeletalMeshComponent* MeshComponent = GetOuterUSkeletalMeshComponent();
	if (MeshComponent)
	{
		// Disable internal delta time and enable external control for rollback compatibility.
		MeshComponent->SetExternalDeltaTime(0);              // Zero internal delta time
		MeshComponent->EnableExternalUpdate(true);           // Enable ECS-driven updates
		MeshComponent->EnableExternalTickRateControl(true);  // Allow ECS tick rate control
	}
}
