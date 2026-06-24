// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "Simulation/Ability/RecallAbilityChainFragments.h"

#include "Data/Ability/RecallAbilityCollectionAsset.h"
#include "GameplayTagsManager.h"

//----------------------------------------------------------------------//
// FRecallAbilityChainInstance
//----------------------------------------------------------------------//
bool FRecallAbilityChainInstance::IsValid() const
{
	if (AbilityCollection)
	{
		return true;
	}
	return false;
}

//----------------------------------------------------------------------//
// FRecallAbilityChainFragment
//----------------------------------------------------------------------//
bool FRecallAbilityChainFragment::Start(const TObjectPtr<const URecallAbilityCollectionAsset>& AbilityCollection)
{
	if (NextAbilityChainCollection == AbilityCollection)
	{
		return false;
	}
	
	if (!AbilityCollection || AbilityCollection->AbilityChain.IsNull())
	{
		return false;
	}
	
	if (ActiveAbilityChainInstance.AbilityCollection == AbilityCollection)
	{
		return false;
	}
	
	NextAbilityChainCollection = AbilityCollection;

	return true;
}

//----------------------------------------------------------------------//
// FRecallAbilityChainInputFragment
//----------------------------------------------------------------------//
TMap<FGameplayTag, int32> FRecallAbilityChainInputFragment::AbilityChainToIndexMap;
FCriticalSection FRecallAbilityChainInputFragment::AbilityChainToIndexMapGuard;

FRecallAbilityChainInputFragment::FRecallAbilityChainInputFragment()
{
	PressedInputTimers.Init(RECALL_ABILITY_CHAIN_BUFFER_MAX_DURATION, MAX_RECALL_ABILITY_CHAIN_INPUT_COUNT);
}

bool FRecallAbilityChainInputFragment::WasAnyInputPressed(int32 InputBufferDuration) const
{
	if (Inputs != 0)
	{
		return true;
	}
	
	for (int32 InputIndex = 0; InputIndex < MAX_RECALL_ABILITY_CHAIN_INPUT_COUNT; InputIndex++)
	{
		if (PressedInputTimers[InputIndex] < InputBufferDuration)
		{
			return true;
		}
	}

	return false;
}

bool FRecallAbilityChainInputFragment::IsInputHeld(const FGameplayTag& AbilityChain) const
{
	const int32 Index = GetAbilityChainIndex(AbilityChain);
	const uint16 InputId = 1 << Index;
	return (Inputs & InputId) != 0;
}

void FRecallAbilityChainInputFragment::PushAbilityChain(const FGameplayTag& AbilityChain)
{
	const int32 Index = GetAbilityChainIndex(AbilityChain);
	const uint16 InputId = 1 << Index;
	InputBuffer |= InputId;
}

bool FRecallAbilityChainInputFragment::WasInputPressed(const FGameplayTag& AbilityChain,
	int32 InputBufferDuration) const
{
	const int32 Index = GetAbilityChainIndex(AbilityChain);
	checkf(FMath::IsWithin(Index, 0, MAX_RECALL_ABILITY_CHAIN_INPUT_COUNT),
		TEXT("%hs Invalid input index"), __FUNCTION__)
	return PressedInputTimers[Index] < InputBufferDuration;
}

void FRecallAbilityChainInputFragment::ClearInputs()
{
	Inputs = 0;
	Direction = FVector2f::ZeroVector;

	for (int32& Timer : PressedInputTimers)
	{
		Timer = RECALL_ABILITY_CHAIN_BUFFER_MAX_DURATION;
	}
}

int32 FRecallAbilityChainInputFragment::GetAbilityChainIndex(const FGameplayTag& AbilityChain)
{
	FScopeLock Lock(&AbilityChainToIndexMapGuard);
	if (AbilityChainToIndexMap.Num() == 0)
	{
		const UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();
	 	const FGameplayTag AbilityChainParentTag = TagsManager.RequestGameplayTag(TEXT("AbilityChain"));
		
		const FGameplayTagContainer AbilityChainTagContainer = TagsManager.RequestGameplayTagChildren(AbilityChainParentTag);

		TArray<FGameplayTag> AbilityChainTags;
		AbilityChainTagContainer.GetGameplayTagArray(AbilityChainTags);

		for (int32 TagIndex = 0; TagIndex < AbilityChainTags.Num(); TagIndex++)
		{
			const FGameplayTag& AbilityChainTag = AbilityChainTags[TagIndex];
			AbilityChainToIndexMap.Add(AbilityChainTag, TagIndex);
		}
	}

	return AbilityChainToIndexMap.FindChecked(AbilityChain);
}
