// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallHitboxEvaluators.h"

#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"
#include "Simulation/Hitbox/RecallHitboxFragments.h"
#include "StateTree/RecallStateTreeExecutionContext.h"

//----------------------------------------------------------------------//
// FRecallExtractHitEvaluator
//----------------------------------------------------------------------//
bool FRecallExtractHitEvaluator::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(HitboxFragmentHandle);
	return true;
}

void FRecallExtractHitEvaluator::TreeStart(FStateTreeExecutionContext& Context) const
{
}

void FRecallExtractHitEvaluator::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{	
	const FRecallHitboxFragment& HitboxFragment = Context.GetExternalData(HitboxFragmentHandle);
	
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);	
	InstanceData.bFoundHit = HitboxFragment.Hits.Num() > 0;
	
	if (InstanceData.bFoundHit)
	{
		InstanceData.Hit = HitboxFragment.Hits[0];
	}
}
