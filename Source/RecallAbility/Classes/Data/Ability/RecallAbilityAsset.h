// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "RecallAbilityAnimationTypes.h"
#include "StructUtils/InstancedStruct.h"

#include "RecallAbilityAsset.generated.h"

UCLASS()
class RECALLABILITY_API URecallAbilityAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category=Animation)
	float BlendIn = 0.f;

	UPROPERTY(EditAnywhere, Category=Animation, meta=(TitleProperty="[{Name}] during [{Duration}]"))
	TArray<FRecallAbilityAnimationSection> AnimationSections;

	/**
	 * Collection of executable operations triggered during ability lifetime.
	 */
	UPROPERTY(EditAnywhere, Category=Ability, meta=(BaseStruct="/Script/RecallAbility.RecallAbilityCommand", ExcludeBaseStruct))
	TArray<FInstancedStruct> Commands;

	/**
	 * Prerequisite logic determining ability activation eligibility.
	 */
	UPROPERTY(EditAnywhere, Category=Ability, meta=(BaseStruct="/Script/RecallAbility.RecallAbilityConditionBase", ExcludeBaseStruct))
	FInstancedStruct Condition;

	/**
	 * Earliest frame permitting transition to subsequent abilities.
	 */
	UPROPERTY(EditAnywhere, Category=Animation, meta=(ClampMin=0), DisplayName="Ability Exit Frame")
	int32 AbilityChainExitFrame = 0;
	
	/**
	 * Gameplay identifiers applied to entity during ability execution.
	 */
	UPROPERTY(EditAnywhere, Category=Animation)
	FGameplayTagContainer AbilityTags;
	
public:
	int32 GetAnimationSectionStartFrame(int32 SectionIndex) const;
	int32 GetDuration() const;
	float GetSectionStartFrame(float Frame, const FRecallAbilityAnimationSection*& OutSection) const;
};
