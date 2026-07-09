// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallProjectileProcessors.h"

#include "Data/Projectile/RecallProjectileTableRow.h"
#include "Kismet/KismetMathLibrary.h"
#include "MassEntityConfigAsset.h"
#include "MassExecutionContext.h"
#include "MassEntityView.h"
#include "RecallSignalSubsystem.h"
#include "Physics/RecallPhysicsObjects.h"
#include "Simulation/Destroy/RecallDestroySignalTypes.h"
#include "Simulation/Hitbox/RecallHitboxFragments.h"
#include "Simulation/Hitbox/RecallHitboxProcessorGroupTypes.h"
#include "Simulation/Physics/RecallPhysicsBodyFragment.h"
#include "Simulation/Physics/RecallPhysicsProcessorGroupTypes.h" 
#include "Simulation/Physics/RecallPhysicsSignalTypes.h"
#include "Simulation/Projectile/RecallProjectileFragments.h"
#include "Simulation/Transform/RecallTransformFragments.h"
#include "System/Entity/RecallEntitySubsystem.h"
#include "System/Hitbox/RecallHitboxSubsystem.h"
#include "System/Physics/RecallPhysicsSubsystem.h"
#include "System/Projectile/RecallProjectileSubsystem.h"
#include "Utility/Math/RecallMathUtils.h"

//----------------------------------------------------------------------//
// URecallProjectileSpawnProcessor
//----------------------------------------------------------------------//
URecallProjectileSpawnProcessor::URecallProjectileSpawnProcessor()
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ProcessingPhase = EMassProcessingPhase::FrameEnd;
}

struct FRecallProjectileRequestCacheManager
{
	FRecallProjectileRequest Request;
	TMap<UMassEntityConfigAsset*, TArray<FRecallProjectileRequest>> RequestMap;

	void ResetCache()
	{
		RequestMap.Reset();
	}
};

void URecallProjectileSpawnProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager)
{
	Super::InitializeInternal(Owner, InEntityManager);

	CacheManager = MakeShared<FRecallProjectileRequestCacheManager>();
}

bool URecallProjectileSpawnProcessor::ShouldAllowQueryBasedPruning(const bool bRuntimeMode /*= true*/) const
{
	return false;
}

void URecallProjectileSpawnProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	ProcessorRequirements.AddSubsystemRequirement<URecallProjectileSubsystem>(EMassFragmentAccess::ReadWrite);
	ProcessorRequirements.AddSubsystemRequirement<URecallEntitySubsystem>(EMassFragmentAccess::ReadWrite);
}

static void SetupProjectile(const FMassEntityView& EntityView, const FRecallProjectileRequest& Request)
{
	if (!ensureAlwaysMsgf(EntityView.HasTag<FRecallProjectileTag>(), TEXT("Spawned non projectile entity as projectile")))
	{
		return;
	}

	FRecallProjectileFragment& ProjectileFragment = EntityView.GetFragmentData<FRecallProjectileFragment>();
	ProjectileFragment.Instigator = Request.Instigator;
	ProjectileFragment.AttackLayers = Request.AttackLayers;
	ProjectileFragment.Projectile = Request.Projectile;
	ProjectileFragment.TargetEntity = Request.Target;

	FRecallTransformFragment& TransformFragment = EntityView.GetFragmentData<FRecallTransformFragment>();
	TransformFragment.Position = Request.SpawnParams.Location;
	TransformFragment.Rotation = UKismetMathLibrary::MakeRotFromX(Request.SpawnParams.Direction).Quaternion();
}

void URecallProjectileSpawnProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	QUICK_SCOPE_CYCLE_COUNTER(Recall_ProjectileSpawn_Execute);
	
	check(CacheManager.IsValid());
	CacheManager->ResetCache();

	FRecallProjectileRequest& Request = CacheManager->Request;
	TMap<UMassEntityConfigAsset*, TArray<FRecallProjectileRequest>>& RequestMap = CacheManager->RequestMap;

	URecallEntitySubsystem& EntitySystem = Context.GetMutableSubsystemChecked<URecallEntitySubsystem>();
	URecallProjectileSubsystem& ProjectileSystem = Context.GetMutableSubsystemChecked<URecallProjectileSubsystem>();

	while (ProjectileSystem.PopProjectilRequest(Request))
	{
		static const FString ContextString(TEXT("URecallProjectileSpawnProcessor::Execute"));
		const FRecallProjectileTableRow* ProjectilePtr = Request.Projectile.GetRow<FRecallProjectileTableRow>(ContextString);
		if (ProjectilePtr == nullptr || !ProjectilePtr->Entity)
		{
			continue;
		}

		TArray<FRecallProjectileRequest>& Requests = RequestMap.FindOrAdd(ProjectilePtr->Entity.Get());
		Requests.Add(Request);
	}
	
	if (RequestMap.Num() == 0)
	{
		return;
	}

	Context.Defer().PushCommand<FMassDeferredCreateCommand>([&EntitySystem, &RequestMap](FMassEntityManager& System)
		{
			TArray<UMassEntityConfigAsset*> EntityConfigs;
			RequestMap.GenerateKeyArray(EntityConfigs);

			for (const UMassEntityConfigAsset* EntityConfig : EntityConfigs)
			{
				const TArray<FRecallProjectileRequest>& Requests = RequestMap.FindChecked(EntityConfig);

				TArray<FMassEntityHandle> Entities;
				EntitySystem.CreateEntities(EntityConfig, Requests.Num(), Entities);

				for (int32 EntityIndex = 0; EntityIndex < Entities.Num(); EntityIndex++)
				{
					const FMassEntityView EntityView(System, Entities[EntityIndex]);
					const FRecallProjectileRequest& Request = Requests[EntityIndex];

					SetupProjectile(EntityView, Request);
				}
			}
		}
	);
}

//----------------------------------------------------------------------//
// URecallProjectileInitializeProcessor
//----------------------------------------------------------------------//
URecallProjectileInitializeProcessor::URecallProjectileInitializeProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ProcessingPhase = EMassProcessingPhase::StartPhysics;
	ExecutionOrder.ExecuteAfter.Add(Recall::Physics::ProcessorGroupNames::Initialize);
	ExecutionOrder.ExecuteBefore.Add(Recall::Physics::ProcessorGroupNames::StartSimulation);
}

void URecallProjectileInitializeProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager)
{
	Super::InitializeInternal(Owner, InEntityManager);
}

void URecallProjectileInitializeProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	FMassTagBitSet InvalidTags;
	InvalidTags.Add(FRecallProjectileInitializedTag::StaticStruct());

	EntityQuery.AddRequirement<FRecallProjectileFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FRecallTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FRecallPhysicsBodyFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddTagRequirements<EMassFragmentPresence::None>(InvalidTags);
	EntityQuery.AddSubsystemRequirement<URecallPhysicsSubsystem>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddSubsystemRequirement<URecallSignalSubsystem>(EMassFragmentAccess::ReadWrite);
}

void URecallProjectileInitializeProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		URecallPhysicsSubsystem& PhysicsSystem = Context.GetMutableSubsystemChecked<URecallPhysicsSubsystem>();
		URecallSignalSubsystem& SignalSystem = Context.GetMutableSubsystemChecked<URecallSignalSubsystem>();

		const TConstArrayView<FRecallProjectileFragment> ProjectileList = Context.GetFragmentView<FRecallProjectileFragment>();
		const TConstArrayView<FRecallPhysicsBodyFragment> BodyList = Context.GetFragmentView<FRecallPhysicsBodyFragment>();
		const TConstArrayView<FRecallTransformFragment> TransformList = Context.GetFragmentView<FRecallTransformFragment>();

		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); EntityIndex++)
		{
			const FMassEntityHandle Entity = Context.GetEntity(EntityIndex);
			const FRecallProjectileFragment& ProjectileFragment = ProjectileList[EntityIndex];

			static const FString ContextString(TEXT("URecallProjectileInitializeProcessor::Execute"));
			const FRecallProjectileTableRow* ProjectilePtr = ProjectileFragment.Projectile.GetRow<FRecallProjectileTableRow>(ContextString);
			if (!ensureAlwaysMsgf(ProjectilePtr != nullptr, TEXT("Invalid projectile")))
			{
				Context.Defer().DestroyEntity(Entity);
				continue;
			}

			const FRecallTransformFragment& TransformFragment = TransformList[EntityIndex];
			const FRecallPhysicsBodyFragment& BodyFragment = BodyList[EntityIndex];

			const FRecallPhysicsBodyView PhysicsBody = PhysicsSystem.GetMutableBody(BodyFragment.BodyHandle);

			if (!ensureMsgf(PhysicsBody.IsValid(), TEXT("Body does not exist.")))
			{
				Context.Defer().DestroyEntity(Entity);
				continue;
			}

			const float SpeedPerFrame = Recall::Math::Utils::UnitsPerSecondToPerFrame(ProjectilePtr->Speed);
			const FVector LinearVelocity = TransformFragment.Rotation.GetForwardVector() * SpeedPerFrame;

			PhysicsBody.AddLinearVelocity(LinearVelocity);

			if (ProjectilePtr->LifeSpan > 0.0f)
			{
				SignalSystem.DelaySignalEntity(Recall::Destroy::Signals::Destroy, Entity, ProjectilePtr->LifeSpan);
			}

			if (ProjectilePtr->bHoming)
			{
				Context.Defer().PushCommand<FMassCommandAddTag<FRecallProjectileHomingTag>>(Entity);
			}
		}

		Context.Defer().PushCommand<FMassCommandAddTag<FRecallProjectileInitializedTag>>(Context.GetEntities());
	});
}

//----------------------------------------------------------------------//
// URecallProjectileHitSignalProcessor
//----------------------------------------------------------------------//
URecallProjectileHitSignalProcessor::URecallProjectileHitSignalProcessor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void URecallProjectileHitSignalProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager)
{
	Super::InitializeInternal(Owner, InEntityManager);

	SubscribeToSignal(Recall::Physics::Signals::Hit);
}

void URecallProjectileHitSignalProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FRecallProjectileFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddConstSharedRequirement<FRecallProjectileConstSharedFragment>();
}

void URecallProjectileHitSignalProcessor::SignalEntities(FMassEntityManager& EntityManager, FMassExecutionContext& Context, FRecallSignalNameLookup& EntitySignals)
{
	QUICK_SCOPE_CYCLE_COUNTER(Recall_ProjectileHit_Signal);

	EntityQuery.ForEachEntityChunk(Context, [&EntitySignals](FMassExecutionContext& Context)
	{		
		const auto& ProjectileConstSharedFragment = Context.GetConstSharedFragment<FRecallProjectileConstSharedFragment>();
		if (ProjectileConstSharedFragment.StateCollection.bDestroyOnHit)
		{
			Context.Defer().DestroyEntities(Context.GetEntities());
		}
		else if (ProjectileConstSharedFragment.StateCollection.bDeactivateAttacksOnHit)
		{
			const TArrayView<FRecallProjectileFragment> ProjectileList = Context.GetMutableFragmentView<FRecallProjectileFragment>();
			
			for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); EntityIndex++)
			{
				FRecallProjectileFragment& ProjectileFragment = ProjectileList[EntityIndex];
				ProjectileFragment.bEnableAttacks = false;
			}
		}
	});
}

//----------------------------------------------------------------------//
// URecallProjectileStateProcessor
//----------------------------------------------------------------------//
URecallProjectileStateProcessor::URecallProjectileStateProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ProcessingPhase = EMassProcessingPhase::PostPhysics;
	ExecutionOrder.ExecuteBefore.Add(Recall::Hitbox::ProcessorGroupNames::PostPhysics::Hitbox);
}

void URecallProjectileStateProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager)
{
	Super::InitializeInternal(Owner, InEntityManager);
}

void URecallProjectileStateProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FRecallTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FRecallProjectileFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddConstSharedRequirement<FRecallProjectileConstSharedFragment>();
	EntityQuery.AddSubsystemRequirement<URecallHitboxSubsystem>(EMassFragmentAccess::ReadWrite);
}

void URecallProjectileStateProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	QUICK_SCOPE_CYCLE_COUNTER(Recall_ProjectileState_Execute);
	
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		URecallHitboxSubsystem& HitboxSystem = Context.GetMutableSubsystemChecked<URecallHitboxSubsystem>();
		
		const auto& ProjectileConstSharedFragment = Context.GetConstSharedFragment<FRecallProjectileConstSharedFragment>();
		
		const TConstArrayView<FRecallTransformFragment> TransformList = Context.GetFragmentView<FRecallTransformFragment>();
		const TConstArrayView<FRecallProjectileFragment> ProjectileList = Context.GetFragmentView<FRecallProjectileFragment>();
		
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); EntityIndex++)
		{
			const FMassEntityHandle Entity = Context.GetEntity(EntityIndex);
			
			const FRecallProjectileFragment& ProjectileFragment = ProjectileList[EntityIndex];
			const FRecallTransformFragment& TransformFragment = TransformList[EntityIndex];

			const FRecallProjectileState* ProjectileStatePtr = ProjectileConstSharedFragment.StateCollection.States.Find(ProjectileFragment.CurrentState);
			if (!ensureAlwaysMsgf(ProjectileStatePtr, TEXT("Invalid projectile state: %s"), *ProjectileFragment.CurrentState.ToString()))
			{
				continue;
			}

			if (ProjectileFragment.bEnableAttacks)
			{
				for (const FRecallAttackDefinition& Attack : ProjectileStatePtr->Attacks)
				{
					HitboxSystem.PushAttack(Entity, TransformFragment.GetTransform(),
						Attack, ProjectileFragment.AttackLayers);
				}
			}
		}
	});
}
