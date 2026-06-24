// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "MassEntityTraitBase.h"

#include "RecallHitboxTraits.generated.h"

/**
* Trait for entities that can take or perform hit on hitboxes.
*/
UCLASS(meta=(DisplayName="MS Hitbox"))
class RECALLHITBOX_API URecallHitboxTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

public:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;

protected:
	/**
	 * Generate a vulnerable hitbox from the collider bounds.
	 * This requires a collider to be attached to this entity.
	 */
	UPROPERTY(EditAnywhere)
	bool bUseColliderBoundsAsHitbox = true;

	/**
	 * Default vulnerable hitboxes of this entity.
	 */
	UPROPERTY(EditAnywhere, meta=(BaseStruct="/Script/RecallHitbox.RecallHitboxDefinitionBase", ExcludeBaseStruct))
	TArray<FInstancedStruct> VulnerableHitboxes;

	/**
	 * Layer vulnerable to attacks from other entities.
	 */
	UPROPERTY(EditAnywhere, meta=(Bitmask, BitmaskEnum="/Script/RecallHitbox.ERecallHitboxLayer"))
	uint8 VulnerableLayers = 1 << 0;

	/**
	 * Default layers that can be affected by an attack from this entity.
	 */
	UPROPERTY(EditAnywhere, meta=(Bitmask, BitmaskEnum="/Script/RecallHitbox.ERecallHitboxLayer"))
	uint8 DefaultAttackLayers = 1 << 1;
};
