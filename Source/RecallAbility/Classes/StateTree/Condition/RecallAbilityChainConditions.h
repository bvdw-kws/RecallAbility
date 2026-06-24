// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "StateTree/RecallAbilityChainStateTreeConditionBase.h"

#include "RecallAbilityChainConditions.generated.h"

USTRUCT()
struct RECALLABILITY_API FRecallAbilityChainInputPressedConditionInstanceData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category=Parameter, meta=(GameplayTagFilter="AbilityChain"))
	FGameplayTag AbilityChain;

	UPROPERTY(EditAnywhere, Category=Parameter, meta=(ClampMin=1, ClampMax=120))
	int32 InputBufferDuration = 1;
};
STATETREE_POD_INSTANCEDATA(FRecallAbilityChainInputPressedConditionInstanceData);

USTRUCT(DisplayName="Was Input Pressed")
struct RECALLABILITY_API FRecallAbilityChainInputPressedCondition :
	public FRecallAbilityChainStateTreeConditionBase
{
	GENERATED_BODY()

	using FInstanceDataType = FRecallAbilityChainInputPressedConditionInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

	UPROPERTY(EditAnywhere, Category="Parameter")
	bool bInvert = false;

protected:
	TStateTreeExternalDataHandle<struct FRecallAbilityChainInputFragment> InputFragmentHandle;
};

USTRUCT()
struct RECALLABILITY_API FRecallAbilityChainInputHeldConditionInstanceData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category=Parameter, meta=(GameplayTagFilter="AbilityChain"))
	FGameplayTag AbilityChain;
};
STATETREE_POD_INSTANCEDATA(FRecallAbilityChainInputHeldConditionInstanceData);

USTRUCT(DisplayName="Is Input Held")
struct RECALLABILITY_API FRecallAbilityChainInputHeldCondition :
	public FRecallAbilityChainStateTreeConditionBase
{
	GENERATED_BODY()

	using FInstanceDataType = FRecallAbilityChainInputHeldConditionInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

	UPROPERTY(EditAnywhere, Category="Parameter")
	bool bInvert = false;

protected:
	TStateTreeExternalDataHandle<struct FRecallAbilityChainInputFragment> InputFragmentHandle;
};

USTRUCT()
struct RECALLABILITY_API FRecallAbilityChainTransitionConditionInstanceData
{
	GENERATED_BODY()
};
STATETREE_POD_INSTANCEDATA(FRecallAbilityChainTransitionConditionInstanceData);

/** 
 * Can transition out of the current ability.
 */
USTRUCT(DisplayName="Can Exit Current Ability")
struct RECALLABILITY_API FRecallAbilityChainTransitionCondition :
	public FRecallAbilityChainStateTreeConditionBase
{
	GENERATED_BODY()

	using FInstanceDataType = FRecallAbilityChainTransitionConditionInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

	UPROPERTY(EditAnywhere, Category="Parameter")
	bool bInvert = false;

protected:
	TStateTreeExternalDataHandle<struct FRecallAbilityFragment> AbilityFragmentHandle;
};
