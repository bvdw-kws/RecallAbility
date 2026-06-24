// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "MassEntityElementTypes.h"
#include "System/AI/RecallStateTreeInstanceTypes.h"
#include "System/Asset/RecallAssetManagerTypes.h"

#include "RecallAbilityChainFragments.generated.h"

class URecallAbilityCollectionAsset;

USTRUCT()
struct RECALLABILITY_API FRecallAbilityChainActiveTag : public FMassTag { GENERATED_BODY() };

USTRUCT()
struct RECALLABILITY_API FRecallAbilityChainInstance
{
	GENERATED_BODY()

	/**
	 * Ability collection used by the current ability chain.
	 */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<const URecallAbilityCollectionAsset> AbilityCollection;
	
	/**
	 * Handle of the state tree.
	 */
	UPROPERTY(VisibleAnywhere)
	FRecallStateTreeInstanceHandle AbilityChainHandle;

	bool IsValid() const;
};

/**
 * Fragment to keep track of the active ability chain.
 */
USTRUCT()
struct RECALLABILITY_API FRecallAbilityChainFragment : public FMassFragment
{
	GENERATED_BODY()

	bool Start(const TObjectPtr<const URecallAbilityCollectionAsset>& AbilityCollection);

	FORCEINLINE TObjectPtr<const URecallAbilityCollectionAsset> GetActiveAbilityCollection() const
	{
		return ActiveAbilityChainInstance.AbilityCollection;
	}

	FORCEINLINE bool IsActive() const
	{
		return ActiveAbilityChainInstance.IsValid() && ActiveAbilityChainInstance.AbilityChainHandle.IsValid();
	}

	/**
	 * Keep track of the ability chain assets that can be used by this entity.
	 */
	UPROPERTY(VisibleAnywhere)
	TMap<FSoftObjectPath, FRecallAssetLoadHandle> AbilityChainAssetMap;
	
	/**
	 * Instance of the active ability chain.
	 */
	UPROPERTY(VisibleAnywhere)
	FRecallAbilityChainInstance ActiveAbilityChainInstance;

	/**
	 * Ability collection of the ability chain waiting to be played.
	 */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<const URecallAbilityCollectionAsset> NextAbilityChainCollection;
	
	/** The last update time use to calculate ticking delta time. */
	UPROPERTY(VisibleAnywhere)
	double LastUpdateTimeInSeconds = 0.;
	
	UPROPERTY(VisibleAnywhere)
	TArray<FMassEntityHandle> TargetEntities;
};

// Defines the longest duration of time an input can be accepted.
constexpr int32 RECALL_ABILITY_CHAIN_BUFFER_MAX_DURATION = 120;

#define MAX_RECALL_ABILITY_CHAIN_INPUT_COUNT 16

USTRUCT()
struct RECALLABILITY_API FRecallAbilityChainInputFragment : public FMassFragment
{
	GENERATED_BODY()

	FRecallAbilityChainInputFragment();
	
	void PushAbilityChain(const FGameplayTag& AbilityChain);

	UPROPERTY(VisibleAnywhere)
	uint16 InputBuffer = 0;
	
	UPROPERTY(VisibleAnywhere)
	uint16 Inputs = 0;
	
	UPROPERTY(VisibleAnywhere)
	TArray<int32> PressedInputTimers;
	
	UPROPERTY(VisibleAnywhere)
	FVector2f Direction = FVector2f::ZeroVector;

	bool IsInputHeld(const FGameplayTag& AbilityChain) const;
	bool WasAnyInputPressed(int32 InputBufferDuration = 1) const;
	bool WasInputPressed(const FGameplayTag& AbilityChain, int32 InputBufferDuration = 1) const;
	void ClearInputs();

private:
	static TMap<FGameplayTag, int32> AbilityChainToIndexMap;
	static FCriticalSection AbilityChainToIndexMapGuard;
	static int32 GetAbilityChainIndex(const FGameplayTag& AbilityChain);
};
