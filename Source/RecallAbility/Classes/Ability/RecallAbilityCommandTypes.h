// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"

#include "RecallAbilityCommandTypes.generated.h"

struct FRecallAbilityExecutionContext;

RECALLABILITY_API DECLARE_LOG_CATEGORY_EXTERN(LogRecallAbilityCommand, Log, All);

enum class RECALLABILITY_API ERecallAbilityExecutionEvent
{
	OnEnter,
	OnExit,
	OnTick,
	OnAnimEnd,
};

USTRUCT()
struct RECALLABILITY_API FRecallAbilityCommand
{
	GENERATED_BODY()
	
	virtual ~FRecallAbilityCommand() = default;
	virtual void Execute(const FRecallAbilityExecutionContext& Context, ERecallAbilityExecutionEvent Event) const {}

public:
	bool EvaluateCondition(const FRecallAbilityExecutionContext& Context) const;

protected:
	UPROPERTY(EditAnywhere, meta=(BaseStruct="/Script/RecallAbility.RecallAbilityConditionBase", ExcludeBaseStruct))
	FInstancedStruct Condition;
};
