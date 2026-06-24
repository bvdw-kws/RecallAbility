// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"

#include "RecallAbilityAnimationTypes.generated.h"

/**
 * Foundation class providing animation asset access interface.
 */
USTRUCT()
struct RECALLABILITY_API FRecallAbilityAnimationWrapperBase
{
	GENERATED_BODY()
	
	virtual ~FRecallAbilityAnimationWrapperBase() = default;

	virtual TObjectPtr<UObject> GetAnimationAsset() const
	{
		unimplemented();
		return nullptr;
	}
};

USTRUCT(BlueprintType, DisplayName="Animation Asset")
struct RECALLABILITY_API FRecallAbilityAnimationAssetWrapper : public FRecallAbilityAnimationWrapperBase
{
	GENERATED_BODY()
	
	/**
	 * Animation resource for sequence playback execution.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimationAsset> Asset;

	virtual TObjectPtr<UObject> GetAnimationAsset() const override
	{
		return Asset;
	}
};

USTRUCT(BlueprintType)
struct RECALLABILITY_API FRecallAbilityAnimationSection
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName Name{ NAME_None };
	
	/**
	 * Animation resource configuration for current segment.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(BaseStruct="/Script/RecallAbility.RecallAbilityAnimationWrapperBase", ExcludeBaseStruct))
	FInstancedStruct Animation;

	/**
	 * Frame boundary specification for animation segment extraction.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FInt32Range AnimFrameRange{ 0, 1 };

	/**
	 * Temporal length of animation segment in discrete frames.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin=0))
	int32 Duration = 0;
	
	/**
	 * Enable cyclical playback upon reaching segment completion.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(InlineEditConditionToggle))
	bool bLoop = false;

	/*
	 * Repetition limit for cyclical animation playback.
	 * Zero indicates unlimited repetition.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition="bLoop", ClampMin=0))
	int32 LoopCount = 0;

	/**
	 * Designated successor segment identifier for sequential playback progression.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName NextSection = NAME_None;

	FORCEINLINE TObjectPtr<UObject> GetAnimationAsset() const
	{
		const FRecallAbilityAnimationWrapperBase* AnimationWrapper = Animation.GetPtr<FRecallAbilityAnimationWrapperBase>();
		return AnimationWrapper != nullptr ? AnimationWrapper->GetAnimationAsset() : nullptr;
	}
};
