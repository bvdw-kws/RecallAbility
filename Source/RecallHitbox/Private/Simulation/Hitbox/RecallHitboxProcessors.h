// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "MassProcessor.h"

#include "RecallHitboxProcessors.generated.h"

/**
 * Clears accumulated attack data at the beginning of each processing cycle.
 * Ensures clean state for collision detection algorithms.
 */
UCLASS()
class URecallResetAttackProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	URecallResetAttackProcessor();

	void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override final;
	bool ShouldAllowQueryBasedPruning(const bool bRuntimeMode = true) const override final;
	
protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override final;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override final;

private:
	FMassEntityQuery EntityQuery;
};

/**
 * Evaluates collision interactions between attack volumes and target entities.
 * Implements spatial testing algorithms for combat system validation.
 */
UCLASS()
class URecallHitboxProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	URecallHitboxProcessor();

	void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override final;
	
protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override final;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override final;

private:
	FMassEntityQuery EntityQuery;
	TSharedPtr<struct FRecallCollisionCacheManager> CacheManager;
};

/**
 * Executes impact consequences when collision validation succeeds.
 * Manages damage application and effect processing workflows.
 */
UCLASS()
class URecallHitProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	URecallHitProcessor();

	void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override final;
	
protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override final;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override final;

private:
	FMassEntityQuery EntityQuery;
};

/**
 * Provides visual debugging for collision system development.
 * Renders collision boundaries and interaction data for analysis.
 */
UCLASS()
class URecallHitboxDebugRepresentationProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	URecallHitboxDebugRepresentationProcessor();

	void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override final;
	
protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override final;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override final;

private:
	FMassEntityQuery EntityQuery;
	TSharedPtr<struct FRecallCollisionCacheManager> CacheManager;
};
