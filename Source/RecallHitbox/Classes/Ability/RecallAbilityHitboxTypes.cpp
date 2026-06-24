// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallAbilityHitboxTypes.h"

#include "Ability/RecallAbilityExecutionTypes.h"
#include "MassExecutionContext.h"
#include "MassEntityView.h"
#include "Simulation/Ability/RecallAbilityFragments.h"
#include "Simulation/Hitbox/RecallHitboxFragments.h"
#include "Simulation/Transform/RecallTransformFragments.h"
#include "System/Hitbox/RecallHitboxSubsystem.h"

/**
 * Add the attack to the list of attacks to be tested against vulnerable entities.
 */
static void PushAttack(const FRecallAbilityExecutionContext& Context, const FRecallAttackDefinition& Attack, bool bOnlyHitTargets)
{
	const FMassEntityView InstigatorView(Context.MassExecutionContext.GetEntityManagerChecked(), Context.Entity);

	FRecallHitboxFragment* HitboxFragmentPtr = InstigatorView.GetFragmentDataPtr<FRecallHitboxFragment>();
	if (!ensureAlwaysMsgf(HitboxFragmentPtr != nullptr, TEXT("Can not perform attack without hit-box trait")))
	{
		return;
	}

	const auto& HitboxConstSharedFragment = InstigatorView.GetConstSharedFragmentData<FRecallHitboxConstSharedFragment>();
	const FRecallTransformFragment& InstigatorTransformFragment = InstigatorView.GetFragmentData<FRecallTransformFragment>();
	
	if (URecallHitboxSubsystem* HitboxSystem = UWorld::GetSubsystem<URecallHitboxSubsystem>(Context.MassExecutionContext.GetWorld()))
	{
		TArray<FMassEntityHandle> Targets;
		
		if (bOnlyHitTargets)
		{
			Targets = Context.AbilityFragment.TargetEntities;
		}

		HitboxSystem->PushAttack(Context.Entity, InstigatorTransformFragment.GetTransform(),
			Attack, HitboxConstSharedFragment.DefaultAttackLayers, Targets);
	}
}

/**
 * Clean the cache for this attack.
 */
static void CleanAttack(const FRecallAbilityExecutionContext& Context, const FRecallAttackDefinition& Attack)
{
	const FMassEntityView EntityView(Context.MassExecutionContext.GetEntityManagerChecked(), Context.Entity);

	FRecallHitboxFragment* HitboxFragmentPtr = EntityView.GetFragmentDataPtr<FRecallHitboxFragment>();
	if (!ensureAlwaysMsgf(HitboxFragmentPtr != nullptr,
		TEXT("%hs Can not perform attack without hit-box trait"), __FUNCTION__))
	{
		return;
	}

	HitboxFragmentPtr->AttackResults.Remove(Attack.AttackID);
}

void FRecallAbilityAttackCommand::Execute(const FRecallAbilityExecutionContext& Context,
                                            ERecallAbilityExecutionEvent Event) const
{
	switch (Event)
	{
	case ERecallAbilityExecutionEvent::OnTick:
		if (FrameRange.Contains(Context.AbilityFragment.ElapsedFrames))
		{
			PushAttack(Context, AttackDefinition, bOnlyHitTargets);
		}
		break;
		
	case ERecallAbilityExecutionEvent::OnExit:
		CleanAttack(Context, AttackDefinition);
		break;

	default:
		break;
	}
}
