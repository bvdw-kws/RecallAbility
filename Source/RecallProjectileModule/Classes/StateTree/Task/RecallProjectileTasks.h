// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"
#include "StateTree/RecallStateTreeTaskBase.h"

#include "RecallProjectileTasks.generated.h"

USTRUCT()
struct RECALLPROJECTILEMODULE_API FRecallSpawnProjectileTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category=Parameter)
	FVector Direction = FVector::ForwardVector;
};

USTRUCT(meta=(DisplayName="Spawn Projectile"))
struct RECALLPROJECTILEMODULE_API FRecallSpawnProjectileTask : public FRecallStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FRecallSpawnProjectileTaskInstanceData;

protected:
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	
	UPROPERTY(EditAnywhere, meta=(RowType="/Script/RecallProjectileModule.RecallProjectileTableRow"))
	FDataTableRowHandle Projectile;
	
	UPROPERTY(EditAnywhere)
	bool bSpawnOnExit = false;

protected:
	TStateTreeExternalDataHandle<struct FRecallTransformFragment> TransformFragmentHandle;
	TStateTreeExternalDataHandle<struct FRecallHitboxConstSharedFragment, EStateTreeExternalDataRequirement::Optional> HitboxConstSharedFragmentHandle;
	TStateTreeExternalDataHandle<class URecallProjectileSubsystem> ProjectileSystemHandle;

private:
	void SpawnProjectile(FStateTreeExecutionContext& Context) const;
};
