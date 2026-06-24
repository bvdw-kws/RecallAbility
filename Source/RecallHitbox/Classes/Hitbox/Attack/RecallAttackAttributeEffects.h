// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "Hitbox/RecallAttackEffectTypes.h"
#include "GameplayTagContainer.h"

#include "RecallAttackAttributeEffects.generated.h"

USTRUCT(DisplayName="Modify Attribute")
struct RECALLHITBOX_API FRecallAttackAttributeModifierEffect : public FRecallAttackEffectBase
{
	GENERATED_BODY()

	virtual void Execute(const FRecallAttackEffectContext& Context) const override;

	UPROPERTY(EditAnywhere, meta=(GameplayTagFilter="Attribute"))
	FGameplayTag Attribute;

	UPROPERTY(EditAnywhere)
	float Value = -1.0f;
};
