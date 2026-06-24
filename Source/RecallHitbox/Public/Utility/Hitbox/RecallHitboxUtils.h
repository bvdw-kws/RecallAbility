// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"

struct FRecallHitboxDefinition;
struct FRecallHitboxTest;

namespace Recall::Hitbox::Utils
{

RECALLHITBOX_API extern void DrawHitbox(const UWorld* World, const FTransform& Transform, const FInstancedStruct& Hitbox, const FColor& Color);
RECALLHITBOX_API extern bool CreateHitboxTest(const FTransform& Transform, const FInstancedStruct& Hitbox, FRecallHitboxTest& OutHitboxTest);
RECALLHITBOX_API extern FRecallHitboxTest CreateHitboxTest(const FTransform& Transform, const FRecallHitboxDefinition& Hitbox);
	
} // namespace Recall::Hitbox::Utils
