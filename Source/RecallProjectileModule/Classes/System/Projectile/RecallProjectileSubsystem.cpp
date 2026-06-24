// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "System/Projectile/RecallProjectileSubsystem.h"

void URecallProjectileSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void URecallProjectileSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void URecallProjectileSubsystem::Reset()
{
}

void URecallProjectileSubsystem::Save(const FRecallSnapshotContext& Context, FInstancedStruct& OutSnapshot)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(TEXT("URecallProjectileSubsystem::Save"));
	QUICK_SCOPE_CYCLE_COUNTER(Recall_Projectile_Save);

	checkf(ProjectileQueue.IsEmpty(), TEXT("Projectile request buffer be emptied before the frame ends"));
}

void URecallProjectileSubsystem::Restore(const FRecallSnapshotContext& Context, const FInstancedStruct& InSnapshot)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR(TEXT("URecallProjectileSubsystem::Restore"));
	QUICK_SCOPE_CYCLE_COUNTER(Recall_Projectile_Restore);
}

void URecallProjectileSubsystem::PushProjectileRequest(const FRecallProjectileRequest& Request)
{
	if (ensureAlwaysMsgf(!Request.Projectile.IsNull(), TEXT("Projectile is null")))
	{
		FScopeLock Lock(&DataGuard);
		ProjectileQueue.Add(Request);
	}	
}

bool URecallProjectileSubsystem::PopProjectilRequest(FRecallProjectileRequest& OutRequest)
{
	FScopeLock Lock(&DataGuard);

	if (ProjectileQueue.IsEmpty())
	{
		return false;
	}

	OutRequest = ProjectileQueue.PopFrontValue();
	return true;
}
