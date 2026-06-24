// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "StateTree/RecallStateTreeEvaluatorBase.h"
#include "Simulation/Hitbox/RecallHitTypes.h"

#include "RecallHitboxEvaluators.generated.h"

USTRUCT()
struct RECALLHITBOX_API FRecallExtractHitEvaluatorInstanceData
{
	GENERATED_BODY()

	/**
	 * True if any hit occured during the past frame.
	 */
	UPROPERTY(VisibleAnywhere, Category=Output)
	bool bFoundHit = false;

	/**
	 * Hit(s) that occured during the past frame.
	 */
	UPROPERTY(VisibleAnywhere, Category=Output, meta=(CanRefToArray))
	FRecallHit Hit;
};

/**
* Extract data from the latest hit(s).
*/
USTRUCT(meta=(DisplayName="Extract Hit"))
struct RECALLHITBOX_API FRecallExtractHitEvaluator : public FRecallStateTreeEvaluatorBase
{
	GENERATED_BODY()

	using FInstanceDataType = FRecallExtractHitEvaluatorInstanceData;

protected:
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual void TreeStart(FStateTreeExecutionContext& Context) const override;
	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

protected:
	TStateTreeExternalDataHandle<struct FRecallHitboxFragment> HitboxFragmentHandle;
};
