// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"
#include "MassEntityView.h"

struct FMassEntityHandle;
struct FMassEntityManager;
struct FMassEntityView;
struct FMassExecutionContext;
struct FRecallAbilityFragment;
struct FRecallGameplayTagFragment;

struct RECALLABILITY_API FRecallAbilityExecutionContext
{
public:
	FMassExecutionContext& MassExecutionContext;
	const FMassEntityHandle& Entity;
	FRecallAbilityFragment& AbilityFragment;
	FRecallGameplayTagFragment* const TagsFragmentPtr = nullptr;

public:
	UWorld* GetWorld() const;
	FMassEntityView GetEntityView() const;
	FMassEntityManager& GetEntityManager() const;
};
