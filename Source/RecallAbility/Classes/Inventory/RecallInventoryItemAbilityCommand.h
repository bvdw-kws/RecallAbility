// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"
#include "Data/Inventory/RecallInventoryItemCommandTypes.h"

#include "RecallInventoryItemAbilityCommand.generated.h"

/**
 * Transition to an ability when the item is used.
 */
USTRUCT(DisplayName="Use Ability")
struct RECALLABILITY_API FRecallInventoryItemAbilityCommand : public FRecallInventoryItemCommand
{
	GENERATED_BODY()

public:
	void OnEnter(const FRecallInventoryItemExecutionContext& Context) const override;
	bool OnTick(const FRecallInventoryItemExecutionContext& Context) const override;
	void OnExit(const FRecallInventoryItemExecutionContext& Context) const override;
	
protected:
	/**
	 * Ability to transition to.
	 */
	UPROPERTY(EditAnywhere)
	TObjectPtr<class URecallAbilityAsset> Ability;
	
	/**
	 * Animation section to start from when entering this ability.
	 */
	UPROPERTY(EditAnywhere)
	FName AnimationSection = NAME_None;
	
	/**
	 * Override the initial animation section based on the equip slot used to trigger this ability.
	 */
	UPROPERTY(EditAnywhere, meta=(GameplayTagFilter="EquipSlot"))
	TMap<FGameplayTag, FName> OverrideEquipSlotAnimSection;
	
	/**
	 * Skip the initial blend-in when entering this animation.
	 */
	UPROPERTY(EditAnywhere)
	bool bIgnoreBlend = false;
};
