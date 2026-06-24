// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "StateTreeEvaluatorBase.h" 

#include "RecallAbilityChainStateTreeEvaluatorBase.generated.h"

/**
 * Base struct for all Recall Ability Chain State Tree Evaluators.
 */
USTRUCT(meta=(DisplayName="Evaluator Base"))
struct RECALLABILITY_API FRecallAbilityChainStateTreeEvaluatorBase : public FStateTreeEvaluatorBase
{
	GENERATED_BODY()
};
