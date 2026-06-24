// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallProjectileHomingProcessors.h"

#include "MassEntityView.h"
#include "MassExecutionContext.h"
#include "RecallSignalSubsystem.h"
#include "Data/Projectile/RecallProjectileTableRow.h"
#include "Kismet/KismetMathLibrary.h"
#include "Physics/RecallPhysicsObjects.h"
#include "Simulation/Physics/RecallPhysicsBodyFragment.h"
#include "Simulation/Projectile/RecallProjectileFragments.h"
#include "Simulation/Transform/RecallTransformFragments.h"
#include "System/Physics/RecallPhysicsSubsystem.h"
#include "Utility/Projectile/RecallProjectileUtils.h"

//----------------------------------------------------------------------//
// URecallProjectileHomingProcessor
//----------------------------------------------------------------------//
URecallProjectileHomingProcessor::URecallProjectileHomingProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = static_cast<int32>(EExtendedProcessorExecutionFlags::All);
	ProcessingPhase = EMassProcessingPhase::StartPhysics;
}

void URecallProjectileHomingProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager)
{
	Super::InitializeInternal(Owner, InEntityManager);
}

void URecallProjectileHomingProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FRecallTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FRecallPhysicsBodyFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FRecallProjectileFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddTagRequirement<FRecallProjectileHomingTag>(EMassFragmentPresence::All);
	EntityQuery.AddConstSharedRequirement<FRecallProjectileConstSharedFragment>();
	EntityQuery.AddSubsystemRequirement<URecallPhysicsSubsystem>(EMassFragmentAccess::ReadWrite);
}

void URecallProjectileHomingProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	QUICK_SCOPE_CYCLE_COUNTER(Recall_ProjectileHoming_Execute);
	
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		const FMassEntityManager& EntityManager = Context.GetEntityManagerChecked();
		const float DeltaTime = Context.GetDeltaTimeSeconds();
		
		URecallPhysicsSubsystem& PhysicsSystem = Context.GetMutableSubsystemChecked<URecallPhysicsSubsystem>();

		const auto& ProjectileConstSharedFragment = Context.GetConstSharedFragment<FRecallProjectileConstSharedFragment>();
		
		const TConstArrayView<FRecallTransformFragment> TransformList = Context.GetFragmentView<FRecallTransformFragment>();
		const TConstArrayView<FRecallPhysicsBodyFragment> BodyList = Context.GetFragmentView<FRecallPhysicsBodyFragment>();
		const TConstArrayView<FRecallProjectileFragment> ProjectileList = Context.GetFragmentView<FRecallProjectileFragment>();
		
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); EntityIndex++)
		{
			const FRecallTransformFragment& TransformFragment = TransformList[EntityIndex];
			const FRecallProjectileFragment& ProjectileFragment = ProjectileList[EntityIndex];
			if (!EntityManager.IsEntityValid(ProjectileFragment.TargetEntity))
			{
				continue;
			}

			static const FString ContextString(TEXT("URecallProjectileHomingProcessor::Execute"));
			const auto* ProjectilePtr = ProjectileFragment.Projectile.GetRow<FRecallProjectileTableRow>(ContextString);
			if (!ensure(ProjectilePtr))
			{
				continue;
			}

			const FRecallPhysicsBodyFragment& BodyFragment = BodyList[EntityIndex];
			const TWeakPtr<FRecallPhysicsBody> Body = PhysicsSystem.GetMutableBody(BodyFragment.BodyHandle);
			if (!Body.IsValid())
			{
				continue;
			}

			const FVector Velocity = Body.Pin()->GetLinearVelocity();
			if (Velocity.IsNearlyZero())
			{
				continue;
			}

			const FVector TargetLocation = Recall::Projectile::Utils::PredictTargetLocation(
				EntityManager, ProjectileFragment.TargetEntity,
				TransformFragment.Position, ProjectilePtr->GetSpeedPerFrame());

			const FVector TargetVector = TargetLocation - TransformFragment.Position;
			if (FVector::DotProduct(Velocity, TargetVector) < 0.0f)
			{
				continue;
			}
			
			FVector Direction = FVector::ZeroVector;
			float Speed = 0.0f;
			Velocity.ToDirectionAndLength(Direction, Speed);

			const FVector TargetDirection = TargetVector.GetSafeNormal(
				UE_SMALL_NUMBER, TransformFragment.Rotation.GetForwardVector());

			const FVector NewDirection = UKismetMathLibrary::VInterpTo(Direction, TargetDirection,
				DeltaTime, ProjectilePtr->HomingInterpSpeed);
			const FVector NewVelocity = NewDirection.GetSafeNormal(UE_SMALL_NUMBER, Direction) * Speed;

			Body.Pin()->SetLinearVelocity(NewVelocity);
		}
	});
}
