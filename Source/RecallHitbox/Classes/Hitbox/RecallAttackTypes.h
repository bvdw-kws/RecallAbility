// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"

#include "RecallAttackTypes.generated.h"

/**
 * Configuration structure defining collision-based attack behavior and properties.
 */
USTRUCT()
struct RECALLHITBOX_API FRecallAttackDefinition
{
	GENERATED_BODY()

	/**
	 * Unique identifier ensuring single active attack registration per entity.
	 * Enables hit counting and prevents duplicate collision processing.
	 */
	UPROPERTY(EditAnywhere, meta=(ClampMin=0, ClampMax=99))
	int32 AttackID = 0;

	/**
	 * Collection of collision volumes for detecting entity interactions.
	 */
	UPROPERTY(EditAnywhere, meta=(BaseStruct="/Script/RecallHitbox.RecallHitboxDefinitionBase", ExcludeBaseStruct))
	TArray<FInstancedStruct> Hitboxes;

	/**
	 * Operations triggered upon successful collision detection and validation.
	 */
	UPROPERTY(EditAnywhere, meta=(BaseStruct="/Script/RecallHitbox.RecallAttackEffectBase", ExcludeBaseStruct))
	TArray<FInstancedStruct> Effects;
};
