// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"
#include "Data/GameplayTag/RecallGameplayTagConditionTypes.h"

#include "RecallAttackEffectTypes.generated.h"

struct FMassEntityHandle;
struct FMassExecutionContext;
struct FRecallGameplayTagFragment;
struct FRecallHit;

struct FRecallAttackEffectContext
{
public:
	FMassExecutionContext& ExecutionContext;
	const FRecallHit& Hit;
	
	// Entity hit
	const FMassEntityHandle& Entity;
	const FRecallGameplayTagFragment* const TagsFragmentPtr = nullptr;

public:
	UWorld* GetWorld() const;
};

/**
 * Effect executed once an attack hit is processed.
 */
USTRUCT()
struct RECALLHITBOX_API FRecallAttackEffectBase
{
	GENERATED_BODY()

public:
	virtual ~FRecallAttackEffectBase() = default;
	virtual void Execute(const FRecallAttackEffectContext& Context) const {}

public:
	bool EvaluateCondition(const FRecallAttackEffectContext& Context) const;
	
protected:
	UPROPERTY(EditAnywhere, meta=(ShowOnlyInnerProperties))
	FRecallGameplayTagCondition GameplayTagCondition;
};
