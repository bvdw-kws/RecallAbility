// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"
#include "Hitbox/RecallAttackTypes.h"

#include "RecallProjectileStateTypes.generated.h"

/**
 * State of the projectile.
 * Each projectile could have a state tree in the future to define his behaviour?
 */
USTRUCT()
struct RECALLPROJECTILEMODULE_API FRecallProjectileState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TArray<FRecallAttackDefinition> Attacks;	
};

USTRUCT()
struct RECALLPROJECTILEMODULE_API FRecallProjectileStateCollection
{
	GENERATED_BODY()
	
	FRecallProjectileStateCollection()
	{
		States.Add(TEXT("Default"), FRecallProjectileState());
	}
	
public:
	/**
	 * Define the states of this projectile.
	 */
	UPROPERTY(EditAnywhere, Category=Projectile, meta=(ShowOnlyInnerProperties))
	TMap<FName, FRecallProjectileState> States;

	/**
	 * Destroy the projectile after it hit a surface.
	 */
	UPROPERTY(EditAnywhere, Category=Projectile)
	bool bDestroyOnHit = true;

	/**
	 * After a hit, the projectile stop pushing attacks so he cannot inflict damage.
	 */
	UPROPERTY(EditAnywhere, Category=Projectile)
	bool bDeactivateAttacksOnHit = false;
};
