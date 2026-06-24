// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "Simulation/Ability/RecallAbilityFragments.h"

#include "Data/Ability/RecallAbilityAsset.h"

bool FRecallAbilityFragment::TransitionToAbility(const URecallAbilityAsset* Ability,
	bool bNewIgnoreBlend /*= false*/, FName Section /*= NAME_None*/, TOptional<TArray<FMassEntityHandle>> Targets /*= TOptional<TArray<FMassEntityHandle>>()*/)
{
	if (IsValid(Ability))
	{
		NextAbility = Ability;
		NextSection = Section;
		bIgnoreBlend = bNewIgnoreBlend;
		TargetEntities = Targets.IsSet() ? Targets.GetValue() : TArray<FMassEntityHandle>();
		bTransitionTriggered = true;
		return true;
	}
	return false;
}
