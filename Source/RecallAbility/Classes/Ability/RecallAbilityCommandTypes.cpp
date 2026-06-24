// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallAbilityCommandTypes.h"

#include "Utility/Ability/RecallAbilityUtils.h"

DEFINE_LOG_CATEGORY(LogRecallAbilityCommand);

//----------------------------------------------------------------------//
// FRecallAbilityCommand
//----------------------------------------------------------------------//
bool FRecallAbilityCommand::EvaluateCondition(const FRecallAbilityExecutionContext& Context) const
{
	return Recall::Ability::Utils::EvaluateAbilityCondition(Condition, Context);
}
