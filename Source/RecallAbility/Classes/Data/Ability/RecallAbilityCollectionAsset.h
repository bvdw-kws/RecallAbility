// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"

#include "RecallAbilityCollectionAsset.generated.h"

UCLASS()
class RECALLABILITY_API URecallAbilityCollectionAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	/**
	 * Collection of abilities per gameplay tag.
	 */
	UPROPERTY(EditAnywhere, meta=(GameplayTagFilter="Ability"))
	TMap<FGameplayTag, TObjectPtr<class URecallAbilityAsset>> Abilities;

	/**
	 * Ability chain that can be used by this ability collection.
	 */
	UPROPERTY(EditAnywhere, meta=(RequiredAssetDataTags="Schema=/Script/RecallAbility.RecallAbilityChainStateTreeSchema"))
	FSoftObjectPath AbilityChain;
};
