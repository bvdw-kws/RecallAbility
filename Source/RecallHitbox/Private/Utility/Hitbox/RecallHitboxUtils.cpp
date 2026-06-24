// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "Utility/Hitbox/RecallHitboxUtils.h"

#include "Hitbox/RecallAttackTypes.h"

namespace Recall::Hitbox::Utils
{
	
bool CreateHitboxTest(const FTransform& Transform, const FInstancedStruct& Hitbox, FRecallHitboxTest& OutHitboxTest)
{
	if (!Hitbox.IsValid())
	{
		return false;
	}

	const FRecallHitboxDefinition* AttackHitboxDefPtr = Hitbox.GetPtr<FRecallHitboxDefinition>();
	if (AttackHitboxDefPtr == nullptr)
	{
		unimplemented();
		return false;
	}

	OutHitboxTest = CreateHitboxTest(Transform, *AttackHitboxDefPtr);
	return true;
}

FRecallHitboxTest CreateHitboxTest(const FTransform& Transform, const FRecallHitboxDefinition& Hitbox)
{
	return FRecallHitboxTest(Transform, Hitbox);
}

void DrawHitbox(const UWorld* World, const FTransform& Transform, const FInstancedStruct& Hitbox, const FColor& Color)
{
	if (!Hitbox.IsValid())
	{
		return;
	}
	
	const FRecallHitboxDefinition* HitboxDefPtr = Hitbox.GetPtr<FRecallHitboxDefinition>();
	if (HitboxDefPtr != nullptr)
	{
		const FVector HitboxCenter = Transform.TransformPositionNoScale(HitboxDefPtr->Offset);		
		DrawDebugBox(World, HitboxCenter, HitboxDefPtr->GetExtent(), Color);
	}
	else
	{
		unimplemented();
	}
}

} // namespace Recall::Hitbox::Utils
