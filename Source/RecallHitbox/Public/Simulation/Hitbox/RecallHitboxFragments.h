// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "MassEntityElementTypes.h"
#include "MassEntityHandle.h"
#include "Simulation/Hitbox/RecallHitTypes.h"

#include "RecallHitboxFragments.generated.h"

USTRUCT()
struct RECALLHITBOX_API FRecallAttackResult
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere)
	TArray<FMassEntityHandle> HitEntities;
};

/**
 * Fragment to hold hitboxes
 */
USTRUCT()
struct RECALLHITBOX_API FRecallHitboxFragment : public FMassFragment
{
	GENERATED_BODY()

	/**
	 * Keep track of the targets hit by each attack.
	 */
	UPROPERTY(VisibleAnywhere)
	TMap<int32, FRecallAttackResult> AttackResults;
	
	/**
	 * List of vulnerable hitboxes that can be hit.
	 */
	UPROPERTY(VisibleAnywhere, meta=(BaseStruct="/Script/RecallHitbox.RecallHitboxDefinitionBase", ExcludeBaseStruct))
	TArray<FInstancedStruct> VulnerableHitboxes;

	/**
	 * Hits taken during the past frame.
	 */
	UPROPERTY(VisibleAnywhere)
	TArray<FRecallHit> Hits;
};

USTRUCT()
struct RECALLHITBOX_API FRecallHitboxConstSharedFragment : public FMassConstSharedFragment
{
	GENERATED_BODY()

	/**
	 * When enabled the collider bounds will be added to the list of vulnerable hitboxes.
	 */
	UPROPERTY(VisibleAnywhere)
	bool bUseColliderBoundsAsHitbox = true;
	
	UPROPERTY(VisibleAnywhere, meta=(Bitmask, BitmaskEnum="/Script/RecallHitbox.ERecallHitboxLayer"))
	uint8 VulnerableLayers = 0;
	
	UPROPERTY(VisibleAnywhere, meta=(Bitmask, BitmaskEnum="/Script/RecallHitbox.ERecallHitboxLayer"))
	uint8 DefaultAttackLayers = 1 << 0;
};
