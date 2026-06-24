// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"

struct FInstancedStruct;
struct FRecallAbilityAnimationFragment;
struct FRecallAbilityExecutionContext;
enum class ERecallAbilityExecutionEvent;
class URecallAbilityAsset;

namespace Recall::Ability::Utils
{

/**
 * Validates prerequisite conditions for ability activation eligibility.
 */
RECALLABILITY_API extern bool EvaluateAbilityCondition(const FInstancedStruct& Condition, const FRecallAbilityExecutionContext& Context);
	
/**
 * Orchestrates state changes between active and queued ability executions.
 */
RECALLABILITY_API extern void ProcessAbilitySequence(
	const FRecallAbilityExecutionContext& AbilityExecutionContext, FRecallAbilityAnimationFragment& AnimationFragment);
	
/**
 * Processes ability operations in response to specific lifecycle events.
 */
RECALLABILITY_API extern void ProcessAbilityCommandSequence(const TObjectPtr<const URecallAbilityAsset>& Ability,
	const FRecallAbilityExecutionContext& AbilityExecutionContext, ERecallAbilityExecutionEvent Event);
	
/**
 * Terminates the active ability and prepares entity for state transition.
 */
RECALLABILITY_API extern void ExitCurrentAbility(const FRecallAbilityExecutionContext& AbilityExecutionContext);
	
/**
 * Navigates to a named animation segment within the ability timeline.
 * Returns success status based on segment availability.
 */
RECALLABILITY_API extern bool NavigateToAnimationSegmentByIdentifier(
	const FRecallAbilityExecutionContext& AbilityExecutionContext, FRecallAbilityAnimationFragment& AnimationFragment, const FName& Name);
	
} // namespace Recall::Ability::Utils
