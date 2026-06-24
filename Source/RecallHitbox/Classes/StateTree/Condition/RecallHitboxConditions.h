// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "StateTree/RecallStateTreeConditionBase.h"
#include "MassEntityTypes.h"

#include "RecallHitboxConditions.generated.h"

USTRUCT()
struct RECALLHITBOX_API FRecallHitConditionInstanceData
{
	GENERATED_BODY()
};
STATETREE_POD_INSTANCEDATA(FRecallHitConditionInstanceData);

USTRUCT(DisplayName="Hit Taken")
struct RECALLHITBOX_API FRecallHitCondition : public FRecallStateTreeConditionBase
{
	GENERATED_BODY()

	using FInstanceDataType = FRecallHitConditionInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

protected:
	UPROPERTY(EditAnywhere, Category=Parameter)
	bool bInvert = false;

private:
	TStateTreeExternalDataHandle<struct FRecallHitboxFragment> HitboxFragmentHandle;
};
