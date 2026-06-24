// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallHitboxConditions.h"

#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"
#include "Simulation/Hitbox/RecallHitboxFragments.h"

//----------------------------------------------------------------------//
// FRecallHitCondition
//----------------------------------------------------------------------//
bool FRecallHitCondition::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(HitboxFragmentHandle);
	return true;
}

bool FRecallHitCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FRecallHitboxFragment& HitboxFragment = Context.GetExternalData(HitboxFragmentHandle);
	const bool bHitTaken = HitboxFragment.Hits.Num() > 0;
	
	return bHitTaken != bInvert;
}
