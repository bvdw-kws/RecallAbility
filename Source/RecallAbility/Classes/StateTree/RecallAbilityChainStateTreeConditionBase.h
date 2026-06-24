// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "StateTreeConditionBase.h" 

#include "RecallAbilityChainStateTreeConditionBase.generated.h"

/**
 * Base struct for all Recall Ability Chain State Tree Conditions.
 */
USTRUCT(meta=(DisplayName="Condition Base"))
struct RECALLABILITY_API FRecallAbilityChainStateTreeConditionBase : public FStateTreeConditionBase
{
	GENERATED_BODY()
};
