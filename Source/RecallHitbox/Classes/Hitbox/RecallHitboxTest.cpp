// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallHitboxTest.h"

#include "Kismet/KismetMathLibrary.h"
#include "RecallHitboxDefinition.h"

FRecallHitboxTest::FRecallHitboxTest(const FTransform& Transform, const FRecallHitboxDefinition& Definition)
{
	const FVector AttackBoxCenter = Transform.TransformPositionNoScale(Definition.Offset);
	Box = FBox(AttackBoxCenter - Definition.GetExtent(), AttackBoxCenter + Definition.GetExtent());
}

bool FRecallHitboxTest::Test(const FRecallHitboxTest& Other, FBox& OutOverlap) const
{
	if (UKismetMathLibrary::Box_Intersects(Box, Other.Box))
	{
		OutOverlap = UKismetMathLibrary::Box_Overlap(Box, Other.Box);
		return true;
	}

	return false;
}
