// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "Ability/RecallAbilityCommandTypes.h"

#include "RecallAbilityPhysicsCommandTypes.generated.h"

USTRUCT(DisplayName="Add Force")
struct RECALLABILITY_API FRecallAbilityAddForceCommand : public FRecallAbilityCommand
{
	GENERATED_BODY()
	
	virtual void Execute(const FRecallAbilityExecutionContext& Context, ERecallAbilityExecutionEvent Event) const override;

protected:
	UPROPERTY(EditAnywhere)
	FInt32Range FrameRange{ 0, 1 };
	
	UPROPERTY(EditAnywhere, meta=(Units=Newtons))
	float Force = 10.0f;
	
	UPROPERTY(EditAnywhere)
	FVector ForceDirection = FVector::UpVector;
};

USTRUCT(DisplayName="Physics Layer")
struct RECALLABILITY_API FRecallAbilityPhysicsLayerCommand : public FRecallAbilityCommand
{
	GENERATED_BODY()
	
	virtual void Execute(const FRecallAbilityExecutionContext& Context, ERecallAbilityExecutionEvent Event) const override;

protected:
	UPROPERTY(EditAnywhere, meta=(RowType="/Script/RecallCore.RecallPhysicsLayerTableRow"))
	FDataTableRowHandle Layer;

};
