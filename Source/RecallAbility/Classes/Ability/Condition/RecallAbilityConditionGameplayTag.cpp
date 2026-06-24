// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallAbilityConditionGameplayTag.h"

#include "Ability/RecallAbilityExecutionTypes.h"
#include "Simulation/GameplayTag/RecallGameplayTagFragments.h"
#include "Utility/GameplayTag/RecallGameplayTagUtils.h"

bool FRecallAbilityConditionGameplayTag::Evaluate(const FRecallAbilityExecutionContext& Context) const
{
	if (Context.TagsFragmentPtr == nullptr)
	{
		return true;
	}

	return Recall::GameplayTag::Utils::EvaluateCondition(GameplayTagCondition, Context.TagsFragmentPtr->GameplayTagCountMap);
}
