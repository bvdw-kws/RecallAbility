// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "System/Hitbox/RecallHitboxSubsystem.h"

#include "System/Hitbox/RecallAttackQueueTypes.h"

void URecallHitboxSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	AttackQueue = MakeShared<FRecallAttackQueue>();
}

void URecallHitboxSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void URecallHitboxSubsystem::Reset()
{
	GetMutableAttackQueue().Reset();
}

void URecallHitboxSubsystem::Save(const FRecallSnapshotContext& Context, FInstancedStruct& OutSnapshot)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(TEXT("URecallHitboxSubsystem::Save"));
	QUICK_SCOPE_CYCLE_COUNTER(Recall_Hitbox_Save);

	checkf(GetMutableAttackQueue().IsEmpty(),
		TEXT("%hs Attacks must be executed the same frame as they are queued"), __FUNCTION__);
}

void URecallHitboxSubsystem::Restore(const FRecallSnapshotContext& Context, const FInstancedStruct& InSnapshot)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(TEXT("URecallHitboxSubsystem::Restore"));
	QUICK_SCOPE_CYCLE_COUNTER(Recall_Hitbox_Restore);
}

FRecallAttackQueue& URecallHitboxSubsystem::GetMutableAttackQueue()
{
	return *AttackQueue.Get();
}

const FRecallAttackQueue& URecallHitboxSubsystem::GetAttackQueue() const
{
	return const_cast<URecallHitboxSubsystem*>(this)->GetMutableAttackQueue();
}

void URecallHitboxSubsystem::PushAttack(const FMassEntityHandle& Instigator, const FTransform& Transform,
	const FRecallAttackDefinition& Attack, uint8 AttackLayers, const TArray<FMassEntityHandle>& Targets)
{
	FRecallAttack& NewAttack = GetMutableAttackQueue().Attacks.AddDefaulted_GetRef();
	NewAttack.Instigator = Instigator;
	NewAttack.AttackLayers = AttackLayers;
	NewAttack.Transform = Transform;
	NewAttack.Definition = Attack;
	NewAttack.Targets = Targets;
}
