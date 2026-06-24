// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "MassEntityElementTypes.h"
#include "MassEntityHandle.h"
#include "Data/Projectile/RecallProjectileStateTypes.h"

#include "RecallProjectileFragments.generated.h"

// Tag to identify projectile entities
USTRUCT() struct RECALLPROJECTILEMODULE_API FRecallProjectileTag : public FMassTag { GENERATED_BODY() };

/** Tag to identify projectile that home at the target. */
USTRUCT() struct RECALLPROJECTILEMODULE_API FRecallProjectileHomingTag : public FMassTag { GENERATED_BODY() };

// Tag to filter projectile entities that have been initialized or not
USTRUCT() struct RECALLPROJECTILEMODULE_API FRecallProjectileInitializedTag : public FMassTag { GENERATED_BODY() };

USTRUCT()
struct RECALLPROJECTILEMODULE_API FRecallProjectileFragment : public FMassFragment
{
	GENERATED_BODY()

	/**
	 * Entity which instigated this projectile.
	 */
	UPROPERTY(VisibleAnywhere)
	FMassEntityHandle Instigator;

	/**
	 * Layers that can be hit by this projectile.
	 */
	UPROPERTY(VisibleAnywhere, meta=(Bitmask, BitmaskEnum="/Script/RecallHitbox.ERecallHitboxLayer"))
	uint8 AttackLayers = 0;

	/**
	 * The state currently active for this projectile.
	 */
	UPROPERTY(VisibleAnywhere)
	FName CurrentState = NAME_None;
	
	/**
	 * Data table from which this projectile was created.
	 */
	UPROPERTY(VisibleAnywhere, meta=(RowType="/Script/RecallProjectileModule.RecallProjectileTableRow"))
	FDataTableRowHandle Projectile;

	/**
	 * Flag to toggle attacks for this projectile.
	 */
	UPROPERTY(VisibleAnywhere)
	bool bEnableAttacks = true;

	/**
	 * Projectile target, if set.
	 */
	UPROPERTY(VisibleAnywhere)
	FMassEntityHandle TargetEntity;
};

USTRUCT()
struct RECALLPROJECTILEMODULE_API FRecallProjectileConstSharedFragment : public FMassConstSharedFragment
{
	GENERATED_BODY()

	/**
	 * Collection of states for this projectile.
	 */
	UPROPERTY(VisibleAnywhere)
	FRecallProjectileStateCollection StateCollection;
};
