// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "Simulation/Hitbox/RecallHitboxTraits.h"

#include "MassEntityTemplateRegistry.h"
#include "Simulation/Hitbox/RecallHitboxFragments.h"
#include "Simulation/Transform/RecallTransformFragments.h"

void URecallHitboxTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);

	BuildContext.RequireFragment<FRecallTransformFragment>();
	
	FRecallHitboxFragment& HitboxFragment = BuildContext.AddFragment_GetRef<FRecallHitboxFragment>();
	HitboxFragment.VulnerableHitboxes.Append(VulnerableHitboxes);
	
	FRecallHitboxConstSharedFragment SharedFragment;
	SharedFragment.bUseColliderBoundsAsHitbox = bUseColliderBoundsAsHitbox;
	SharedFragment.VulnerableLayers = VulnerableLayers;
	SharedFragment.DefaultAttackLayers = DefaultAttackLayers;

	BuildContext.AddConstSharedFragment(EntityManager.GetOrCreateConstSharedFragment(SharedFragment));
}
