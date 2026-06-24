// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "RecallSignalProcessorBase.h"

#include "RecallProjectileProcessors.generated.h"

UCLASS()
class URecallProjectileSpawnProcessor : public UMassProcessor
{
	GENERATED_BODY()

	URecallProjectileSpawnProcessor();

public:
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override;
	virtual bool ShouldAllowQueryBasedPruning(const bool bRuntimeMode = true) const override;

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	TSharedPtr<struct FRecallProjectileRequestCacheManager> CacheManager;
};

UCLASS()
class URecallProjectileInitializeProcessor : public UMassProcessor
{
	GENERATED_BODY()

	URecallProjectileInitializeProcessor();

public:
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override;

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery EntityQuery;

	TSharedPtr<struct FRecallProjectileRequestCacheManager> CacheManager;
};

UCLASS()
class URecallProjectileHitSignalProcessor : public URecallSignalProcessorBase
{
	GENERATED_BODY()

public:
	URecallProjectileHitSignalProcessor(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override final;

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override final;
	virtual void SignalEntities(FMassEntityManager& EntityManager, FMassExecutionContext& Context, FRecallSignalNameLookup& EntitySignals) override final;
};

UCLASS()
class URecallProjectileStateProcessor : public UMassProcessor
{
	GENERATED_BODY()

	URecallProjectileStateProcessor();

public:
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override;

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery EntityQuery;
};
