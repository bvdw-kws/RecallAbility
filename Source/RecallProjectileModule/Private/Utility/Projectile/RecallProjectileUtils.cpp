// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "Utility/Projectile/RecallProjectileUtils.h"

#include "Data/Projectile/RecallProjectileTableRow.h"
#include "MassEntityManager.h"
#include "MassEntityView.h"
#include "Physics/RecallPhysicsObjects.h"
#include "Simulation/Physics/RecallPhysicsBodyFragment.h"
#include "Simulation/Transform/RecallTransformFragments.h"
#include "System/Physics/RecallPhysicsSubsystem.h"

namespace Recall::Projectile::Utils
{

FVector PredictTargetLocation(const FVector& TargetLocation, const FVector& TargetVelocity,
	 const FVector& ProjectileLocation, float ProjectileSpeed, int32 MaxIteration)
{
	if (ProjectileSpeed > 0.0f)
	{
		float AverageHitDuration = 0.0f;

		for (int32 i = 0; i < FMath::Max(1, MaxIteration); i++)
		{
			const FVector NewTargetLocation = TargetLocation + TargetVelocity * AverageHitDuration;
			const FVector TargetVector = NewTargetLocation - ProjectileLocation;

			FVector TargetDirection = FVector::ZeroVector;
			float TargetDistance = 0.0f;
			TargetVector.ToDirectionAndLength(TargetDirection, TargetDistance);

			const float HitDuration = TargetDistance / ProjectileSpeed;
			if (i > 0)
			{
				AverageHitDuration = (AverageHitDuration + HitDuration) * 0.5f;
			}
			else
			{
				AverageHitDuration = HitDuration;				
			}
		}

		return TargetLocation + TargetVelocity * AverageHitDuration;
	}
	else
	{
		return TargetLocation;
	}
}

FVector PredictTargetLocation(const FMassEntityManager& EntityManager, const FMassEntityHandle& TargetEntity,
	const FVector& ProjectileLocation, float ProjectileSpeed, int32 MaxIteration)
{
	const FMassEntityView TargetView(EntityManager, TargetEntity);
	const auto& TargetTransformFragment = TargetView.GetFragmentData<FRecallTransformFragment>();

	FVector TargetVelocity = FVector::ZeroVector;

	if (const auto* BodyFragmentPtr = TargetView.GetFragmentDataPtr<FJPRPhysicsBodyFragment>())
	{
		auto* PhysicsSystem = UWorld::GetSubsystem<URecallPhysicsSubsystem>(EntityManager.GetWorld());
		check(PhysicsSystem);
		const FJPRPhysicsBodyView Body = PhysicsSystem->GetMutableBody(BodyFragmentPtr->BodyHandle);
		if (Body.IsValid())
		{
			TargetVelocity = Body.GetLinearVelocity();
		}
	}

	return PredictTargetLocation(TargetTransformFragment.Position, TargetVelocity,
		ProjectileLocation, ProjectileSpeed);
}

FVector GetAimAtTargetDirection(
	const FMassEntityManager& EntityManager, const FMassEntityHandle& TargetEntity,
	const FVector& ProjectileLocation, const FDataTableRowHandle& Projectile, int32 MaxIteration)
{
	float ProjectileSpeed = 0.0f;

	static const FString ContextString(TEXT("Recall::Projectile::Utils::GetAimAtTargetDirection"));			
	if (const auto* ProjectileRow = Projectile.GetRow<FRecallProjectileTableRow>(ContextString))
	{
		ProjectileSpeed = ProjectileRow->GetSpeedPerFrame();
	}

	const FVector TargetLocation = PredictTargetLocation(EntityManager, TargetEntity,
		ProjectileLocation, ProjectileSpeed);

	return (TargetLocation - ProjectileLocation).GetSafeNormal();
}

} // namespace Recall::Projectile::Utils
