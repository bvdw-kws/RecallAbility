// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"

#include "RecallAbilityConditionTypes.generated.h"

struct FRecallAbilityExecutionContext;

USTRUCT()
struct RECALLABILITY_API FRecallAbilityConditionBase
{
	GENERATED_BODY()
	
	virtual ~FRecallAbilityConditionBase() = default;
	virtual bool Evaluate(const FRecallAbilityExecutionContext& Context) const;
};

USTRUCT(DisplayName="And")
struct RECALLABILITY_API FRecallAbilityConditionAnd : public FRecallAbilityConditionBase
{
	GENERATED_BODY()
	
	virtual bool Evaluate(const FRecallAbilityExecutionContext& Context) const override;

protected:
	UPROPERTY(EditAnywhere, meta=(BaseStruct="/Script/RecallAbility.RecallAbilityConditionBase", ExcludeBaseStruct))
	TArray<FInstancedStruct> Conditions;
};
