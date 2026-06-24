// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "MassProcessor.h"

#include "RecallAbilityAnimationProcessors.generated.h"

/**
* This processor handles ability animations
*/
UCLASS()
class RECALLABILITY_API URecallAbilityAnimationProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	URecallAbilityAnimationProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override final;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override final;

private:
	FMassEntityQuery EntityQuery;
	
};

/**
* This processor handles synchronization of animation state before display
*/
UCLASS()
class RECALLABILITY_API URecallAbilityAnimationSynchronizationProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	URecallAbilityAnimationSynchronizationProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override final;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override final;

private:
	FMassEntityQuery EntityQuery;
	
};

/**
* This processor manage ability animation representation.
*/
UCLASS()
class RECALLABILITY_API URecallAbilityAnimationSkeletalMeshRepresentationProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	URecallAbilityAnimationSkeletalMeshRepresentationProcessor();

	void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override final;
	
protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override final;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override final;

private:
	FMassEntityQuery EntityQuery;
};
