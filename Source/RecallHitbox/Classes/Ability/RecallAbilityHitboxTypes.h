// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "Ability/RecallAbilityCommandTypes.h"
#include "Hitbox/RecallAttackTypes.h"

#include "RecallAbilityHitboxTypes.generated.h"

USTRUCT(DisplayName="Attack")
struct RECALLHITBOX_API FRecallAbilityAttackCommand : public FRecallAbilityCommand
{
	GENERATED_BODY()

public:	
	virtual void Execute(const FRecallAbilityExecutionContext& Context, ERecallAbilityExecutionEvent Event) const override;

protected:
	UPROPERTY(EditAnywhere)
	FInt32Range FrameRange{ 0, 1 };

	UPROPERTY(EditAnywhere)
	FRecallAttackDefinition AttackDefinition;
		
	UPROPERTY(EditAnywhere)
	bool bOnlyHitTargets = false;
};
