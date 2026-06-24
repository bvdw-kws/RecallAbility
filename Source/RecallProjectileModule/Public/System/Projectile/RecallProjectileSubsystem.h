// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "System/Interface/RecallSimulationReactSystemInterface.h"
#include "MassExternalSubsystemTraits.h"
#include "MassEntityHandle.h"
#include "Containers/RingBuffer.h"
#include "Simulation/Projectile/RecallProjectileTypes.h"

#include "RecallProjectileSubsystem.generated.h"

USTRUCT()
struct RECALLPROJECTILEMODULE_API FRecallProjectileRequest
{
	GENERATED_BODY()

	/**
	 * Entity which instigated the projectile.
	 */
	UPROPERTY(VisibleAnywhere)
	FMassEntityHandle Instigator;
	
	/**
	 * Data table row defining the projectile entity.
	 */
	UPROPERTY(VisibleAnywhere, meta=(RowType="/Script/RecallProjectileModule.RecallProjectileTableRow"))
	FDataTableRowHandle Projectile;

	/**
	 * Layers that can be hit by the projectile.
	 */
	UPROPERTY(VisibleAnywhere, meta=(Bitmask, BitmaskEnum="/Script/RecallHitbox.ERecallHitboxLayer"))
	uint8 AttackLayers = 0;

	/**
	 * Spawn parameters for the projectile.
	 */
	UPROPERTY(VisibleAnywhere)
	FRecallProjectileSpawnParameters SpawnParams;

	/**
	 * Projectile target, if set.
	 */
	UPROPERTY(VisibleAnywhere)
	FMassEntityHandle Target;
};

UCLASS()
class RECALLPROJECTILEMODULE_API URecallProjectileSubsystem :
	public UWorldSubsystem,
	public IRecallSimulationReactSystemInterface
{
	GENERATED_BODY()

public:
	void PushProjectileRequest(const FRecallProjectileRequest& SpawnParams);
	bool PopProjectilRequest(FRecallProjectileRequest& OutRequest);

protected:
	// UWorldSubsystem implementation Begin
	void Initialize(FSubsystemCollectionBase& Collection) override final;
	void Deinitialize() override final;
	// UWorldSubsystem implementation End

	// IRecallSimulationReactSystemInterface implementation Begin
	void Reset() override final;
	void Save(const FRecallSnapshotContext& Context, FInstancedStruct& OutSnapshot) override final;
	void Restore(const FRecallSnapshotContext& Context, const FInstancedStruct& InSnapshot) override final;
	// IRecallSimulationReactSystemInterface implementation End

private:
	mutable FCriticalSection DataGuard;
	TRingBuffer<FRecallProjectileRequest, TFixedAllocator<64>> ProjectileQueue;

};

template<>
struct TMassExternalSubsystemTraits<URecallProjectileSubsystem> final
{
	enum
	{
		GameThreadOnly = false
	};
};
