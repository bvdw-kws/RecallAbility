// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallAbilityConditionTypes.h"

//----------------------------------------------------------------------//
// FRecallAbilityConditionBase
//----------------------------------------------------------------------//
bool FRecallAbilityConditionBase::Evaluate(const FRecallAbilityExecutionContext& Context) const
{
	unimplemented();
	return false;
}

//----------------------------------------------------------------------//
// FRecallAbilityConditionAnd
//----------------------------------------------------------------------//
bool FRecallAbilityConditionAnd::Evaluate(const FRecallAbilityExecutionContext& Context) const
{
	for (const FInstancedStruct& Condition : Conditions)
	{
		if (!Condition.IsValid())
		{
			continue;
		}

		const FRecallAbilityConditionBase* ConditionPtr = Condition.GetPtr<FRecallAbilityConditionBase>();
		if (ConditionPtr == nullptr)
		{
			continue;
		}

		if (!ConditionPtr->Evaluate(Context))
		{
			return false;
		}
	}

	return true;
}
