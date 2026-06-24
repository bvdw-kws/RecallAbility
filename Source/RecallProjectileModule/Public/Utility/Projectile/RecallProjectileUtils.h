// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"

struct FMassEntityManager;
struct FMassEntityHandle;
struct FMassEntityView;

namespace Recall::Projectile::Utils
{

RECALLPROJECTILEMODULE_API extern FVector PredictTargetLocation(
	const FVector& TargetLocation, const FVector& TargetVelocity,
		const FVector& ProjectileLocation, float ProjectileSpeed, int32 MaxIteration = 3);	
RECALLPROJECTILEMODULE_API extern FVector PredictTargetLocation(
	const FMassEntityManager& EntityManager, const FMassEntityHandle& TargetEntity,
		const FVector& ProjectileLocation, float ProjectileSpeed, int32 MaxIteration = 3);
	
RECALLPROJECTILEMODULE_API extern FVector GetAimAtTargetDirection(
	const FMassEntityManager& EntityManager, const FMassEntityHandle& TargetEntity,
		const FVector& ProjectileLocation, const FDataTableRowHandle& Projectile, int32 MaxIteration = 3);

} // namespace Recall::Projectile::Utils
