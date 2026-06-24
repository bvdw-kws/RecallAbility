// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "Ability/RecallAbilityCommandTypes.h"

#include "RecallAbilityProjectileTypes.generated.h"

struct FMassEntityHandle;

USTRUCT(DisplayName="Projectile")
struct RECALLPROJECTILEMODULE_API FRecallAbilityProjectileCommand : public FRecallAbilityCommand
{
	GENERATED_BODY()

public:	
	virtual void Execute(const FRecallAbilityExecutionContext& Context, ERecallAbilityExecutionEvent Event) const override;
	
protected:
	/**
	 * Frame of the ability to which the projectile should be spawned.
	 */
	UPROPERTY(EditAnywhere, meta=(ClampMin=0))
	int32 SpawnFrame = 0;

	/**
	 * Whether to use the control rotation instead of the instigator rotation to orient the projectile. 
	 */
	UPROPERTY(EditAnywhere)
	bool bUseControlRotation = true;
	
	/**
	 * Spawn location offset of the projectile relative to the instigator.
	 */
	UPROPERTY(EditAnywhere)
	FVector SpawnOffset = FVector::ZeroVector;

	/**
	 * Spawn direction of the projectile relative to the instigator.
	 */
	UPROPERTY(EditAnywhere)
	FVector SpawnDirection = FVector::ForwardVector;

	/**
	 * Definition of the projectile to spawn.
	 */
	UPROPERTY(EditAnywhere, meta=(RowType="/Script/RecallProjectileModule.RecallProjectileTableRow"))
	FDataTableRowHandle Projectile;

	/**
	 * Maximum dispersion angle of the projectile.
	 */
	UPROPERTY(EditAnywhere, meta=(Units=Degrees, ClampMin=0, ClampMax=90))
	float MaxDispersionAngle = 0.0f;

private:
	FMassEntityHandle GetTargetEntity(const FRecallAbilityExecutionContext& Context) const;
	FVector GetSpawnDirection(const FRecallAbilityExecutionContext& Context,
		const FVector& SpawnLocation, const FMassEntityHandle& TargetEntity) const;
	FTransform GetInstigatorTransform(const FRecallAbilityExecutionContext& Context) const;
};
