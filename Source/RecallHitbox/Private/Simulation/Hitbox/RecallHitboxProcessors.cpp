// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallHitboxProcessors.h"

#include "Hitbox/RecallAttackEffectTypes.h"
#include "Hitbox/RecallHitboxDefinition.h"
#include "Hitbox/RecallHitboxTest.h"
#include "Hitbox/RecallHitboxTypes.h"
#include "MassEntityView.h"
#include "MassExecutionContext.h"
#include "RecallSignalSubsystem.h"
#include "Simulation/GameplayTag/RecallGameplayTagFragments.h"
#include "Simulation/Hitbox/RecallHitboxProcessorGroupTypes.h"
#include "Simulation/Hitbox/RecallHitboxFragments.h"
#include "Simulation/Physics/RecallPhysicsBodyFragment.h"
#include "Simulation/StateTree/RecallStateTreeSignalTypes.h"
#include "Simulation/Transform/RecallTransformFragments.h"
#include "System/Hitbox/RecallAttackQueueTypes.h"
#include "System/Hitbox/RecallHitboxSubsystem.h"
#include "Utility/Hitbox/RecallHitboxUtils.h"

//----------------------------------------------------------------------//
// URecallHitboxProcessor
//----------------------------------------------------------------------//
URecallHitboxProcessor::URecallHitboxProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ProcessingPhase = EMassProcessingPhase::PostPhysics;
	ExecutionOrder.ExecuteInGroup = Recall::Hitbox::ProcessorGroupNames::PostPhysics::Hitbox;
}

struct FHitboxCollisionCache
{
	FRecallHitboxTest CollisionTest;
};

struct FTargetEntityCollisionData
{
	FMassEntityHandle EntityHandle;
	uint8 InteractionLayers = 0;
	TArray<FHitboxCollisionCache> CollisionVolumes;

	FORCEINLINE void ClearCollisionData()
	{
		CollisionVolumes.Reset();
	}
};

struct FRecallCollisionCacheManager
{
	int32 ActiveEntityCount = 0;
	TArray<FTargetEntityCollisionData> TrackedEntities;
	
	FORCEINLINE void RefreshCollisionState()
	{
		for (int32 EntityIndex = 0; EntityIndex < ActiveEntityCount; EntityIndex++)
		{
			TrackedEntities[EntityIndex].ClearCollisionData();
		}
		ActiveEntityCount = 0;
	}
};

void URecallHitboxProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager)
{
	Super::InitializeInternal(Owner, InEntityManager);

	CacheManager = MakeShared<FRecallCollisionCacheManager>();
}

void URecallHitboxProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FRecallTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FRecallHitboxFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FRecallPhysicsBodyFragment>(EMassFragmentAccess::ReadWrite, EMassFragmentPresence::Optional);
	EntityQuery.AddConstSharedRequirement<FRecallHitboxConstSharedFragment>();
	
	ProcessorRequirements.AddSubsystemRequirement<URecallSignalSubsystem>(EMassFragmentAccess::ReadWrite);
	ProcessorRequirements.AddSubsystemRequirement<URecallHitboxSubsystem>(EMassFragmentAccess::ReadWrite);
}

struct FRecallExecuteAttackContext
{
	FMassExecutionContext& ExecutionContext;
	const FRecallAttack& Attack;
	const FRecallHitboxTest& AttackHitboxTest;
	const FTargetEntityCollisionData& VulnerableTarget;
};

static bool ExecuteAttack(const FRecallExecuteAttackContext& Context)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(TEXT("URecallHitboxProcessor::ExecuteAttack"));
	QUICK_SCOPE_CYCLE_COUNTER(Recall_Hitbox_ExecuteAttack);
	
	// Do not hit self.
	if (Context.VulnerableTarget.EntityHandle == Context.Attack.Instigator)
	{
		return false;
	}

	// Only hit selected targets if any
	if (!Context.Attack.Targets.IsEmpty() && !Context.Attack.Targets.Contains(Context.VulnerableTarget.EntityHandle))
	{
		return false;
	}
	
	const FMassEntityManager& EntityManager = Context.ExecutionContext.GetEntityManagerChecked();
	const FVector AttackDirection = Context.Attack.Transform.GetRotation().GetForwardVector();
	
	// Test against vulnerable hitboxes.
	for (const FHitboxCollisionCache& VulnerableHitbox : Context.VulnerableTarget.CollisionVolumes)
	{
		// Detect a hit.
		FBox BoxOverlap;
		if (!Context.AttackHitboxTest.Test(VulnerableHitbox.CollisionTest, BoxOverlap))
		{
			continue;
		}

		if (EntityManager.IsEntityValid(Context.Attack.Instigator))
		{
			const FMassEntityView AttackInstigatorView(EntityManager, Context.Attack.Instigator);
			FRecallHitboxFragment* AttackInstigatorHitboxFragmentPtr = AttackInstigatorView.GetFragmentDataPtr<FRecallHitboxFragment>();
			if (AttackInstigatorHitboxFragmentPtr != nullptr)
			{
				// Prevent the attack from hitting the same entity twice.
				const int32 AttackID = Context.Attack.Definition.AttackID;
				FRecallAttackResult& AttackResult = AttackInstigatorHitboxFragmentPtr->AttackResults.FindOrAdd(AttackID);
				if (AttackResult.HitEntities.Contains(Context.VulnerableTarget.EntityHandle))
				{
					break;
				}
		
				AttackResult.HitEntities.Add(Context.VulnerableTarget.EntityHandle);
			}
		}

		const FMassEntityView VulnerableTargetView(EntityManager, Context.VulnerableTarget.EntityHandle);
		FRecallHitboxFragment& VulnerableHitboxFragment = VulnerableTargetView.GetFragmentData<FRecallHitboxFragment>();
		
		// Register the hit on the hit entity.
		VulnerableHitboxFragment.Hits.Add(FRecallHit{
			Context.Attack.Instigator, Context.Attack.Definition, BoxOverlap.GetCenter(), AttackDirection
			}
		);
		return true;
	}

	return false;
}

void URecallHitboxProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	QUICK_SCOPE_CYCLE_COUNTER(Recall_Hitbox_Execute);

	check(CacheManager.IsValid());
	CacheManager->RefreshCollisionState();

	int32& VulnerableEntityCount = CacheManager->ActiveEntityCount;
	TArray<FTargetEntityCollisionData>& VulnerableEntities = CacheManager->TrackedEntities;
	
	// Store our vulnerable hitboxes	
	EntityQuery.ForEachEntityChunk(Context,
		[&VulnerableEntityCount, &VulnerableEntities](FMassExecutionContext& Context)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR(TEXT("URecallHitboxProcessor::ExecuteAttack GatherHitboxes"));
		QUICK_SCOPE_CYCLE_COUNTER(Recall_Hitbox_GatherHitboxes);
			
		const FRecallHitboxConstSharedFragment& HitboxConstSharedFragment = Context.GetConstSharedFragment<FRecallHitboxConstSharedFragment>();

		const TArrayView<FRecallHitboxFragment> HitboxList = Context.GetMutableFragmentView<FRecallHitboxFragment>();
		const TConstArrayView<FRecallTransformFragment> TransformList = Context.GetFragmentView<FRecallTransformFragment>();
		const TConstArrayView<FRecallPhysicsBodyFragment> BodyList = Context.GetFragmentView<FRecallPhysicsBodyFragment>();

		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); EntityIndex++)
		{
			const FRecallTransformFragment& TransformFragment = TransformList[EntityIndex];
			
			FRecallHitboxFragment& HitboxFragment = HitboxList[EntityIndex];
			HitboxFragment.Hits.Reset();

			if (VulnerableEntities.Num() == VulnerableEntityCount)
			{
				VulnerableEntities.AddDefaulted();
			}

			// Cache vulnerable hitboxes
			FTargetEntityCollisionData& VulnerableEntityCache = VulnerableEntities[VulnerableEntityCount++];
			VulnerableEntityCache.EntityHandle = Context.GetEntity(EntityIndex);
			VulnerableEntityCache.InteractionLayers = HitboxConstSharedFragment.VulnerableLayers;
			
			const FTransform VulnerableTransform = TransformFragment.GetTransform();

			for (const FInstancedStruct& Hitbox : HitboxFragment.VulnerableHitboxes)
			{
				FRecallHitboxTest HitboxTest;
				if (Recall::Hitbox::Utils::CreateHitboxTest(VulnerableTransform, Hitbox, HitboxTest))
				{
					FHitboxCollisionCache& HitboxCache = VulnerableEntityCache.CollisionVolumes.AddDefaulted_GetRef();
					HitboxCache.CollisionTest = HitboxTest;
				}
			}

			// Use entity collider as a vulnerable hit-box
			if (HitboxConstSharedFragment.bUseColliderBoundsAsHitbox)
			{
				const FRecallPhysicsBodyFragment& BodyFragment = BodyList[EntityIndex];
				
				FHitboxCollisionCache& HitboxCache = VulnerableEntityCache.CollisionVolumes.AddDefaulted_GetRef();
				HitboxCache.CollisionTest = Recall::Hitbox::Utils::CreateHitboxTest(
					VulnerableTransform, FRecallHitboxDefinition(BodyFragment.Extents));
			}
		}
	});
	
	URecallHitboxSubsystem& HitboxSystem = Context.GetMutableSubsystemChecked<URecallHitboxSubsystem>();
	URecallSignalSubsystem& SignalSystem = Context.GetMutableSubsystemChecked<URecallSignalSubsystem>();
			
	// Test attacks
	for (const FRecallAttack& Attack : HitboxSystem.GetMutableAttackQueue().Attacks)
	{
		for (const FInstancedStruct& Hitbox : Attack.Definition.Hitboxes)
		{
			FRecallHitboxTest AttackHitboxTest;
			if (!Recall::Hitbox::Utils::CreateHitboxTest(Attack.Transform, Hitbox, AttackHitboxTest))
			{
				continue;
			}

			const ERecallHitboxLayer AttackLayers = static_cast<ERecallHitboxLayer>(Attack.AttackLayers);
			
			for (int32 TargetIndex = 0; TargetIndex < VulnerableEntityCount; TargetIndex++)
			{
				const FTargetEntityCollisionData& TargetEntityCache = VulnerableEntities[TargetIndex];
				const ERecallHitboxLayer VulnerableLayers = static_cast<ERecallHitboxLayer>(TargetEntityCache.InteractionLayers);

				if (!EnumHasAnyFlags(VulnerableLayers, AttackLayers))
				{
					continue;
				}
			
				const FRecallExecuteAttackContext ExecuteAttackContext{
					Context, Attack, AttackHitboxTest, TargetEntityCache
				};
				if (ExecuteAttack(ExecuteAttackContext))
				{						
					SignalSystem.SignalEntity(Recall::StateTree::Signals::TickRequired, TargetEntityCache.EntityHandle);
				}
			}
		}
	}
}

//----------------------------------------------------------------------//
// URecallHitProcessor
//----------------------------------------------------------------------//
URecallHitProcessor::URecallHitProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ProcessingPhase = EMassProcessingPhase::PostPhysics;
	ExecutionOrder.ExecuteInGroup = Recall::Hitbox::ProcessorGroupNames::PostPhysics::Hit;
	ExecutionOrder.ExecuteAfter.Add(Recall::Hitbox::ProcessorGroupNames::PostPhysics::Hitbox);
}

void URecallHitProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager)
{
	Super::InitializeInternal(Owner, InEntityManager);
}

void URecallHitProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FRecallHitboxFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FRecallGameplayTagFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	EntityQuery.RegisterWithProcessor(*this);	
}

void URecallHitProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	QUICK_SCOPE_CYCLE_COUNTER(Recall_Hit_Execute);

	// Execute hit effects	
	EntityQuery.ForEachEntityChunk(Context,
		[](FMassExecutionContext& Context)
	{
		const TConstArrayView<FRecallHitboxFragment> HitboxList = Context.GetFragmentView<FRecallHitboxFragment>();
		const TConstArrayView<FRecallGameplayTagFragment> TagsList = Context.GetFragmentView<FRecallGameplayTagFragment>();

		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); EntityIndex++)
		{
			const FRecallHitboxFragment& HitboxFragment = HitboxList[EntityIndex];
			if (HitboxFragment.Hits.Num() == 0)
			{
				continue;
			}

			const FMassEntityHandle Entity = Context.GetEntity(EntityIndex);
			const FRecallGameplayTagFragment* TagsFragmentPtr = TagsList.IsValidIndex(EntityIndex) ? &TagsList[EntityIndex] : nullptr;
			
			for (const FRecallHit& Hit : HitboxFragment.Hits)
			{
				const FRecallAttackEffectContext EffectContext{
					Context, Hit, Entity, TagsFragmentPtr
				};
				
				for (const FInstancedStruct& Effect : Hit.Attack.Effects)
				{
					const FRecallAttackEffectBase* EffectPtr = Effect.GetPtr<FRecallAttackEffectBase>();
					if (EffectPtr == nullptr || !EffectPtr->EvaluateCondition(EffectContext))
					{
						continue;
					}

					EffectPtr->Execute(EffectContext);
				}
			}
		}
	});
}

//----------------------------------------------------------------------//
// URecallResetAttackProcessor
//----------------------------------------------------------------------//
URecallResetAttackProcessor::URecallResetAttackProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ProcessingPhase = EMassProcessingPhase::PostPhysics;
	ExecutionOrder.ExecuteInGroup = Recall::Hitbox::ProcessorGroupNames::PrePhysics::ResetAttack;
	ExecutionOrder.ExecuteAfter.Add(Recall::Hitbox::ProcessorGroupNames::PostPhysics::Hitbox);
}

void URecallResetAttackProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager)
{
	Super::InitializeInternal(Owner, InEntityManager);
}

bool URecallResetAttackProcessor::ShouldAllowQueryBasedPruning(const bool bRuntimeMode) const
{
	return false;
}

void URecallResetAttackProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	ProcessorRequirements.AddSubsystemRequirement<URecallHitboxSubsystem>(EMassFragmentAccess::ReadWrite);
}

void URecallResetAttackProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	QUICK_SCOPE_CYCLE_COUNTER(Recall_ResetAttack_Execute);
	
	URecallHitboxSubsystem& HitboxSystem = Context.GetMutableSubsystemChecked<URecallHitboxSubsystem>();
	HitboxSystem.GetMutableAttackQueue().Reset();
}

//----------------------------------------------------------------------//
// URecallHitboxDebugRepresentationProcessor
//----------------------------------------------------------------------//
#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
static bool bDebugRecallHitboxShowAll = false;
static FAutoConsoleVariableRef CVarRecallHitboxShowAll(
	TEXT("Recall.Hitbox.ShowAll"),
	bDebugRecallHitboxShowAll,
	TEXT("Show Hitboxes")
);
#endif // UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT

URecallHitboxDebugRepresentationProcessor::URecallHitboxDebugRepresentationProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ProcessingPhase = EMassProcessingPhase::Render;
	bRequiresGameThreadExecution = true;
}

void URecallHitboxDebugRepresentationProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager)
{
	Super::InitializeInternal(Owner, InEntityManager);
}

void URecallHitboxDebugRepresentationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	EntityQuery.AddRequirement<FRecallTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FRecallHitboxFragment>(EMassFragmentAccess::ReadOnly);	
	EntityQuery.AddRequirement<FRecallPhysicsBodyFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	EntityQuery.AddConstSharedRequirement<FRecallHitboxConstSharedFragment>();	

	ProcessorRequirements.AddSubsystemRequirement<URecallHitboxSubsystem>(EMassFragmentAccess::ReadOnly);
#endif // UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
}

void URecallHitboxDebugRepresentationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	QUICK_SCOPE_CYCLE_COUNTER(Recall_Hitbox_Representation);

	if (!bDebugRecallHitboxShowAll)
	{
		return;
	}

	const URecallHitboxSubsystem& HitboxSystem = Context.GetSubsystemChecked<URecallHitboxSubsystem>();
	
	for (const FRecallAttack& Attack : HitboxSystem.GetAttackQueue().DebugAttacks)
	{
		for (const FInstancedStruct& Hitbox : Attack.Definition.Hitboxes)
		{
			Recall::Hitbox::Utils::DrawHitbox(Context.GetWorld(), Attack.Transform, Hitbox, FColor::Blue);
		}
	}
	
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		const FRecallHitboxConstSharedFragment& HitboxConstSharedFragment = Context.GetConstSharedFragment<FRecallHitboxConstSharedFragment>();
		
		const TConstArrayView<FRecallTransformFragment> TransformList = Context.GetFragmentView<FRecallTransformFragment>();
		const TConstArrayView<FRecallHitboxFragment> HitboxList = Context.GetFragmentView<FRecallHitboxFragment>();
		const TConstArrayView<FRecallPhysicsBodyFragment> BodyList = Context.GetFragmentView<FRecallPhysicsBodyFragment>();
		
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); EntityIndex++)
		{
			const FRecallTransformFragment& TransformFragment = TransformList[EntityIndex];
			const FRecallHitboxFragment& HitboxFragment = HitboxList[EntityIndex];

			for (const FInstancedStruct& VulnerableHitbox : HitboxFragment.VulnerableHitboxes)
			{
				Recall::Hitbox::Utils::DrawHitbox(Context.GetWorld(), TransformFragment.GetTransform(), VulnerableHitbox, FColor::Red);
			}

			if (HitboxConstSharedFragment.bUseColliderBoundsAsHitbox && BodyList.IsValidIndex(EntityIndex))
			{
				const FRecallPhysicsBodyFragment& BodyFragment = BodyList[EntityIndex];
				const FInstancedStruct Hitbox = FInstancedStruct::Make<FRecallHitboxDefinition>(BodyFragment.Extents);
				
				Recall::Hitbox::Utils::DrawHitbox(Context.GetWorld(), TransformFragment.GetTransform(), Hitbox, FColor::Red);
			}
		}
	});
#endif // UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
}
