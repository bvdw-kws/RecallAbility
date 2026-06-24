// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"
#include "Hitbox/RecallAttackTypes.h"
#include "MassEntityHandle.h"

struct RECALLHITBOX_API FRecallAttack
{
	/**
	 * Entity who instigated the attack.
	 */
	FMassEntityHandle Instigator;

	/**
	 * Layers that can be hit by this attack.
	 */
	uint8 AttackLayers = 0;

	/**
	 * Transform of the attack.
	 */
	FTransform Transform = FTransform::Identity;

	/**
	 * Definition of this attack.
	 */
	FRecallAttackDefinition Definition;

	/**
	 * Only hit the targets in this list, or any target if empty.
	 */
	TArray<FMassEntityHandle> Targets;
};

struct RECALLHITBOX_API FRecallAttackQueue
{
	/**
	 * Attacks to process before the end of the current frame.
	 */
    TArray<FRecallAttack> Attacks;
	
#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	/**
	 * Attacks that were executed during the last frame.
	 */
    TArray<FRecallAttack> DebugAttacks;
#endif // UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT

	FORCEINLINE void Reset()
	{
#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
		DebugAttacks.Reset(Attacks.Num());
		DebugAttacks.Append(Attacks);
#endif // UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
		
		Attacks.Reset();
	}
	
	FORCEINLINE bool IsEmpty() const
	{
		return Attacks.IsEmpty();
	}
};
