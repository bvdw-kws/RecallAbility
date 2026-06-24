// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "MassObserverProcessor.h"
#include "RecallSignalProcessorBase.h"

#include "RecallAbilityChainProcessors.generated.h"

UCLASS()
class URecallAbilityChainDestructor : public UMassObserverProcessor
{
	GENERATED_BODY()

	URecallAbilityChainDestructor();

protected:
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override;
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};

UCLASS()
class RECALLABILITY_API URecallAbilityChainInputProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	URecallAbilityChainInputProcessor();

	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override final;
	
protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override final;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override final;

private:
	FMassEntityQuery EntityQuery;
};

/**
* This processor updates the active ability chains.
*/
UCLASS()
class RECALLABILITY_API URecallAbilityChainProcessor : public URecallSignalProcessorBase
{
	GENERATED_BODY()

public:
	URecallAbilityChainProcessor(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override final;

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override final;
	virtual void SignalEntities(FMassEntityManager& EntityManager, FMassExecutionContext& Context, FRecallSignalNameLookup& EntitySignals) override final;
};

UCLASS()
class RECALLABILITY_API URecallAbilityChainDebugRepresentationProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	URecallAbilityChainDebugRepresentationProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override final;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override final;

private:
	FMassEntityQuery EntityQuery;
};
