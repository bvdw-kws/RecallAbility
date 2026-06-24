// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallAbilityProjectileTypes.h"

#include "Ability/RecallAbilityExecutionTypes.h"
#include "MassEntityView.h"
#include "MassExecutionContext.h"
#include "Simulation/Ability/RecallAbilityFragments.h"
#include "Simulation/Controller/RecallControllerFragments.h"
#include "Simulation/Hitbox/RecallHitboxFragments.h"
#include "Simulation/Transform/RecallTransformFragments.h"
#include "System/Projectile/RecallProjectileSubsystem.h"
#include "System/Random/RecallRandomNumberSubsystem.h"
#include "Utility/Projectile/RecallProjectileUtils.h"

//----------------------------------------------------------------------//
// FRecallAbilityProjectileCommand
//----------------------------------------------------------------------//
void FRecallAbilityProjectileCommand::Execute(const FRecallAbilityExecutionContext& Context,
                                            ERecallAbilityExecutionEvent Event) const
{
	if (Event != ERecallAbilityExecutionEvent::OnTick || SpawnFrame != Context.AbilityFragment.ElapsedFrames)
	{
		return;
	}

	const FMassEntityHandle TargetEntity = GetTargetEntity(Context);
	
	FRecallProjectileSpawnParameters SpawnParams;
	SpawnParams.Location = GetInstigatorTransform(Context).TransformPositionNoScale(SpawnOffset);
	SpawnParams.Direction = GetSpawnDirection(Context, SpawnParams.Location, TargetEntity);	

	FRecallProjectileRequest ProjectileRequest;
	ProjectileRequest.Instigator = Context.Entity;
	ProjectileRequest.Projectile = Projectile;
	ProjectileRequest.SpawnParams = SpawnParams;
	ProjectileRequest.Target = TargetEntity;

	const FMassEntityView InstigatorView(Context.GetEntityManager(), Context.Entity);
	if (const auto* HitboxConstSharedFragmentPtr = InstigatorView.GetConstSharedFragmentDataPtr<FRecallHitboxConstSharedFragment>())
	{
		ProjectileRequest.AttackLayers = HitboxConstSharedFragmentPtr->DefaultAttackLayers;
	}
	else
	{
		UE_LOG(LogRecallAbilityCommand, Warning,
			TEXT("%hs The projectile instigator does not have a hitbox trait attached"), __FUNCTION__)
	}
	
	if (URecallProjectileSubsystem* ProjectileSystem = UWorld::GetSubsystem<URecallProjectileSubsystem>(Context.MassExecutionContext.GetWorld()))
	{
		ProjectileSystem->PushProjectileRequest(ProjectileRequest);
	}
}

FMassEntityHandle FRecallAbilityProjectileCommand::GetTargetEntity(
	const FRecallAbilityExecutionContext& Context) const
{
	// TODO: Target selection?
	if (Context.AbilityFragment.TargetEntities.Num() > 0)
	{
		return Context.AbilityFragment.TargetEntities[0];
	}

	return FMassEntityHandle();
}

FVector FRecallAbilityProjectileCommand::GetSpawnDirection(const FRecallAbilityExecutionContext& Context,
	const FVector& SpawnLocation, const FMassEntityHandle& TargetEntity) const
{
	FVector Result = SpawnDirection;
	
	const FMassEntityManager& EntityManager = Context.GetEntityManager();
	if (EntityManager.IsEntityValid(TargetEntity))
	{
		Result = Recall::Projectile::Utils::GetAimAtTargetDirection(EntityManager, TargetEntity, SpawnLocation, Projectile);
	}
	else
	{
		const FTransform InstigatorTransform = GetInstigatorTransform(Context);
		Result = InstigatorTransform.TransformVectorNoScale(SpawnDirection).GetSafeNormal();
	}

	if (MaxDispersionAngle > 0.0f)
	{
		auto* RandomNumberSystem = UWorld::GetSubsystem<URecallRandomNumberSubsystem>(Context.GetWorld());
		check(RandomNumberSystem);
		const FRandomStream& RandomStream = RandomNumberSystem->GetRandomStream();
		Result = RandomStream.VRandCone(Result, MaxDispersionAngle);
	}

	return Result;
}

FTransform FRecallAbilityProjectileCommand::GetInstigatorTransform(
	const FRecallAbilityExecutionContext& Context) const
{
	const FMassEntityManager& EntityManager = Context.GetEntityManager();
	const FMassEntityView InstigatorView(EntityManager, Context.Entity);
	const auto& InstigatorTransformFragment = InstigatorView.GetFragmentData<FRecallTransformFragment>();

	FVector InstigatorLocation = InstigatorTransformFragment.Position;
	FQuat InstigatorRotation = InstigatorTransformFragment.Rotation;

	if (bUseControlRotation)
	{
		if (const auto* ControllerFragmentPtr = InstigatorView.GetFragmentDataPtr<FRecallControllerFragment>())
		{
			InstigatorRotation = ControllerFragmentPtr->ControlRotation.Quaternion();
		}
	}
		
	const FTransform InstigatorTransform(InstigatorRotation, InstigatorLocation);
	return InstigatorTransform;
}
