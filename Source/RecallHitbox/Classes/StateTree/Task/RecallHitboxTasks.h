// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "StateTree/RecallStateTreeTaskBase.h"
#include "Simulation/Hitbox/RecallHitTypes.h"

#include "RecallHitboxTasks.generated.h"

USTRUCT(BlueprintType)
struct RECALLHITBOX_API FRecallLocalizedHit
{
	GENERATED_BODY()

	/**
	 * Location where the hit happened.
	 */
	UPROPERTY(VisibleAnywhere)
	FVector Location = FVector::ZeroVector;

	/**
	 * Nearest EquipSlot that was hit.
	 */
	UPROPERTY(VisibleAnywhere, meta=(GameplayTagFilter="EquipSlot"))
	FGameplayTag EquipSlot;
};

USTRUCT()
struct RECALLHITBOX_API FRecallLocalizeHitTaskInstanceData
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, Category=Input, meta=(CanRefToArray))
	FRecallHit Hit;

	UPROPERTY(VisibleAnywhere, Category=Output, meta=(CanRefToArray))
	FRecallLocalizedHit LocalizedHit;
};

USTRUCT(meta=(DisplayName="Localize Hit"))
struct RECALLHITBOX_API FRecallLocalizeHitTask : public FRecallStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FRecallLocalizeHitTaskInstanceData;

protected:
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

	UPROPERTY(EditAnywhere)
	bool bSucceedOnDone = false;

private:
	TStateTreeExternalDataHandle<struct FRecallEquipmentConstSharedFragment> EquipmentConstSharedFragmentHandle;
	TStateTreeExternalDataHandle<struct FRecallEquipmentFragment> EquipmentFragmentHandle;
	
	TArray<FRecallLocalizedHit> ExtractLocalizedHit(FStateTreeExecutionContext& Context, const TArray<FRecallHit>& Hits) const;
	
};
