// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"

#include "RecallHitboxTypes.generated.h"

UENUM(meta=(BitFlags, UseEnumValuesAsMaskValuesInEditor="true"))
enum class ERecallHitboxLayer : uint8
{
	None				= 0,

	Layer1				= 1 << 0	UMETA(DisplayName="Team 1"),
	Layer2				= 1 << 1	UMETA(DisplayName="Team 2"),
	Layer3				= 1 << 2	UMETA(DisplayName="Team 3"),
	Layer4				= 1 << 3	UMETA(DisplayName="Team 4"),
};
ENUM_CLASS_FLAGS(ERecallHitboxLayer)
