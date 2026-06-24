// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallAbilityChainConditions.h"

#include "Data/Ability/RecallAbilityAsset.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"
#include "Simulation/Ability/RecallAbilityChainFragments.h"
#include "Simulation/Ability/RecallAbilityFragments.h"

//----------------------------------------------------------------------//
// FRecallAbilityChainInputPressedCondition
//----------------------------------------------------------------------//
bool FRecallAbilityChainInputPressedCondition::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(InputFragmentHandle);
	return true;
}

bool FRecallAbilityChainInputPressedCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	const FRecallAbilityChainInputFragment& InputFragment = Context.GetExternalData(InputFragmentHandle);
	const int32 InputBufferDuration = FMath::Max(1, InstanceData.InputBufferDuration);
	
	return InputFragment.WasInputPressed(
		InstanceData.AbilityChain, InputBufferDuration) != bInvert;
}

//----------------------------------------------------------------------//
// FRecallAbilityChainInputHeldCondition
//----------------------------------------------------------------------//
bool FRecallAbilityChainInputHeldCondition::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(InputFragmentHandle);
	return true;
}

bool FRecallAbilityChainInputHeldCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	const FRecallAbilityChainInputFragment& InputFragment = Context.GetExternalData(InputFragmentHandle);
	
	return InputFragment.IsInputHeld(InstanceData.AbilityChain) != bInvert;
}

//----------------------------------------------------------------------//
// FRecallAbilityChainTransitionCondition
//----------------------------------------------------------------------//
bool FRecallAbilityChainTransitionCondition::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(AbilityFragmentHandle);
	return true;
}

bool FRecallAbilityChainTransitionCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FRecallAbilityFragment& AbilityFragment = Context.GetExternalData(AbilityFragmentHandle);

	const bool bCanTransition = !AbilityFragment.CurrentAbility ||
		AbilityFragment.ElapsedFrames >= AbilityFragment.CurrentAbility->AbilityChainExitFrame;
	
	return bCanTransition != bInvert;
}
