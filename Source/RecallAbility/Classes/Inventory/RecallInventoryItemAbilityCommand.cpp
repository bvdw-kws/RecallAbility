// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallInventoryItemAbilityCommand.h"

#include "MassEntityView.h"
#include "MassExecutionContext.h"
#include "Simulation/Ability/RecallAbilityFragments.h"

void FRecallInventoryItemAbilityCommand::OnEnter(const FRecallInventoryItemExecutionContext& Context) const
{	
	const FMassEntityView EntityView(Context.MassExecutionContext.GetEntityManagerChecked(), Context.Entity);
	FRecallAbilityFragment* AbilityFragmentPtr = EntityView.GetFragmentDataPtr<FRecallAbilityFragment>();
	if (!ensureAlwaysMsgf(AbilityFragmentPtr != nullptr, TEXT("Entity does not have ability trait")))
	{
		return;
	}

	if (!ensureAlwaysMsgf(Ability, TEXT("Ability is not set")))
	{
		return;
	}

	const FName* OverrideAnimSectionPtr = OverrideEquipSlotAnimSection.Find(Context.EquipSlot);
	const FName Section = OverrideAnimSectionPtr != nullptr ? *OverrideAnimSectionPtr : AnimationSection;
	
	AbilityFragmentPtr->TransitionToAbility(Ability, bIgnoreBlend, Section);
}

bool FRecallInventoryItemAbilityCommand::OnTick(const FRecallInventoryItemExecutionContext& Context) const
{
	if (!ensureAlwaysMsgf(Ability, TEXT("Ability is not set")))
	{
		return false;
	}
	
	const FMassEntityView EntityView(Context.MassExecutionContext.GetEntityManagerChecked(), Context.Entity);
	FRecallAbilityFragment* AbilityFragmentPtr = EntityView.GetFragmentDataPtr<FRecallAbilityFragment>();
	if (!ensureAlwaysMsgf(AbilityFragmentPtr != nullptr, TEXT("Entity does not have ability trait")))
	{
		return false;
	}

	// Wait until ability is done
	return AbilityFragmentPtr->CurrentAbility == Ability;
}

void FRecallInventoryItemAbilityCommand::OnExit(const FRecallInventoryItemExecutionContext& Context) const
{
}
