// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "Base/RecallHitboxDefinitionBase.h"

#include "RecallHitboxDefinition.generated.h"

USTRUCT(BlueprintType, DisplayName="Hitbox")
struct RECALLHITBOX_API FRecallHitboxDefinition : public FRecallHitboxDefinitionBase
{
	GENERATED_BODY()

	FRecallHitboxDefinition() = default;
	FRecallHitboxDefinition(const FVector& Bounds) : Width(Bounds.X), Height(Bounds.Z), Depth(Bounds.Y) {}

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Units=Centimeters))
	float Width = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly,meta=(Units=Centimeters))
	float Height = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Units=Centimeters))
	float Depth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector Offset = FVector::ZeroVector;
	
	FORCEINLINE FVector GetExtent() const { return FVector(Width, Depth, Height); }
};
