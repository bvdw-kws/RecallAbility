// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"
#include "Ability/RecallAbilityConditionTypes.h"
#include "Data/GameplayTag/RecallGameplayTagConditionTypes.h"

#include "RecallAbilityConditionGameplayTag.generated.h"

USTRUCT(DisplayName="Gameplay Tags")
struct RECALLABILITY_API FRecallAbilityConditionGameplayTag : public FRecallAbilityConditionBase
{
	GENERATED_BODY()
	
public:
	bool Evaluate(const FRecallAbilityExecutionContext& Context) const override;

protected:
	UPROPERTY(EditAnywhere, meta=(ShowOnlyInnerProperties))
	FRecallGameplayTagCondition GameplayTagCondition;
};
