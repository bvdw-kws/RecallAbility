// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallProjectileTasks.h"

#include "Simulation/Transform/RecallTransformFragments.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"
#include "Simulation/Hitbox/RecallHitboxFragments.h"
#include "StateTree/RecallStateTreeExecutionContext.h"
#include "System/Projectile/RecallProjectileSubsystem.h"

//----------------------------------------------------------------------//
// FRecallSpawnProjectileTask
//----------------------------------------------------------------------//
bool FRecallSpawnProjectileTask::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(TransformFragmentHandle);
	Linker.LinkExternalData(HitboxConstSharedFragmentHandle);
	Linker.LinkExternalData(ProjectileSystemHandle);
	return true;
}

void FRecallSpawnProjectileTask::SpawnProjectile(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	const FRecallStateTreeExecutionContext& MassContext = static_cast<FRecallStateTreeExecutionContext&>(Context);

	const FRecallTransformFragment& TransformFragment = Context.GetExternalData(TransformFragmentHandle);

	FRecallProjectileSpawnParameters SpawnParams;
	SpawnParams.Location = TransformFragment.Position;
	SpawnParams.Direction = InstanceData.Direction;

	FRecallProjectileRequest ProjectileRequest;
	ProjectileRequest.Instigator = MassContext.GetEntity();
	ProjectileRequest.Projectile = Projectile;
	ProjectileRequest.SpawnParams = SpawnParams;

	if (const FRecallHitboxConstSharedFragment* HitboxConstSharedFragmentPtr = Context.GetExternalDataPtr(
			HitboxConstSharedFragmentHandle))
	{
		ProjectileRequest.AttackLayers = HitboxConstSharedFragmentPtr->DefaultAttackLayers;
	}
	else
	{
		UE_LOG(LogTemp, Warning,
			TEXT("%hs The projectile instigator does not have a hitbox trait attached"), __FUNCTION__)
	}
	
	URecallProjectileSubsystem& ProjectileSystem = Context.GetExternalData(ProjectileSystemHandle);
	ProjectileSystem.PushProjectileRequest(ProjectileRequest);
}

EStateTreeRunStatus FRecallSpawnProjectileTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	if (!bSpawnOnExit)
	{
		SpawnProjectile(Context);
		return EStateTreeRunStatus::Succeeded;
	}

	return Super::EnterState(Context, Transition);
}

void FRecallSpawnProjectileTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	if (bSpawnOnExit)
	{
		SpawnProjectile(Context);
	}

	return Super::ExitState(Context, Transition);
}

EStateTreeRunStatus FRecallSpawnProjectileTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	return Super::Tick(Context, DeltaTime);
}
