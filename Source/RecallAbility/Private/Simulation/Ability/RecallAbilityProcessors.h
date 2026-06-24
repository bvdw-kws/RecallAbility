// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "MassProcessor.h"

#include "RecallAbilityProcessors.generated.h"

/**
 * Manages the execution and state transitions of abilities within the ECS framework.
 * Processes ability timing, animation events, and state changes each frame.
 */
UCLASS()
class RECALLABILITY_API URecallAbilityProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	URecallAbilityProcessor();

	void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override final;
	
protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override final;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override final;

private:
	FMassEntityQuery EntityQuery;
};

/**
 * Provides debug visualization and rendering support for ability system data.
 * Displays real-time ability information for development and debugging purposes.
 */
UCLASS()
class RECALLABILITY_API URecallAbilityRepresentationProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	URecallAbilityRepresentationProcessor();

	void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override final;
	
protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override final;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override final;

private:
	FMassEntityQuery EntityQuery;
};
