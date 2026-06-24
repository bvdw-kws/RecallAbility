// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "Simulation/Projectile/RecallProjectileTraits.h"

#include "MassEntityTemplateRegistry.h"
#include "Simulation/Projectile/RecallProjectileFragments.h"
#include "Simulation/Transform/RecallTransformFragments.h"

void URecallProjectileTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);

	BuildContext.RequireFragment<FRecallTransformFragment>();

	FRecallProjectileFragment& ProjectileFragment = BuildContext.AddFragment_GetRef<FRecallProjectileFragment>();
	ProjectileFragment.CurrentState = DefaultState;

	FRecallProjectileConstSharedFragment SharedFragment;
	SharedFragment.StateCollection = StateCollection;

	BuildContext.AddConstSharedFragment(EntityManager.GetOrCreateConstSharedFragment(SharedFragment));
	
	BuildContext.AddTag<FRecallProjectileTag>();
}
