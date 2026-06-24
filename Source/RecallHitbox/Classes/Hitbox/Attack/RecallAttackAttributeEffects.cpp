// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallAttackAttributeEffects.h"

#include "MassEntityView.h"
#include "MassExecutionContext.h"
#include "Simulation/Attribute/RecallAttributeFragments.h"

void FRecallAttackAttributeModifierEffect::Execute(const FRecallAttackEffectContext& Context) const
{	
	if (!ensureAlwaysMsgf(Attribute.IsValid(),
		TEXT("%hs Attribute modify is missing attribute"), __FUNCTION__))
	{
		return;
	}
	
	const FMassEntityManager& EntityManager = Context.ExecutionContext.GetEntityManagerChecked();
	const FMassEntityView EntityView(EntityManager, Context.Entity);
	
	FRecallAttributeFragment* AttributeFragmentPtr = EntityView.GetFragmentDataPtr<FRecallAttributeFragment>();
	if (AttributeFragmentPtr == nullptr)
	{
		return;
	}

	AttributeFragmentPtr->AttributeSet.ModifyValue(Attribute, Value);
}
