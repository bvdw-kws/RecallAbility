// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "StateTree/RecallStateTreeTaskBase.h"
#include "System/Asset/RecallAssetManagerTypes.h"

#include "RecallAbilityTasks.generated.h"

class URecallAbilityAsset;
class URecallAbilityCollectionAsset;

UENUM()
enum class ERecallAbilityTaskSource : uint8
{
	Asset,
	Collection,
};

USTRUCT()
struct RECALLABILITY_API FRecallAbilityTaskInstanceData
{
	GENERATED_BODY()

	/**
	 * Entity collection designated as potential ability recipients.
	 */
	UPROPERTY(EditAnywhere, Category=Parameter)
	TArray<FMassEntityHandle> Targets;
	
	UPROPERTY(EditAnywhere, Category=Parameter)
	FStateTreeDelegateDispatcher OnAbilityFinishedDelegate;

	UPROPERTY()
	bool bFinishedAbility = false;
};

USTRUCT(meta=(DisplayName="Ability"))
struct RECALLABILITY_API FRecallAbilityTask : public FRecallStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FRecallAbilityTaskInstanceData;

protected:
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	
	UPROPERTY(EditAnywhere, DisplayName="Source")
	ERecallAbilityTaskSource AbilitySource = ERecallAbilityTaskSource::Asset;
	
	UPROPERTY(EditAnywhere, meta=(EditCondition="AbilitySource == ERecallAbilityTaskSource::Asset", EditConditionHides))
	TObjectPtr<URecallAbilityAsset> Ability;
	
	UPROPERTY(EditAnywhere, meta=(EditCondition="AbilitySource == ERecallAbilityTaskSource::Collection", EditConditionHides, GameplayTagFilter="Ability"))
	FGameplayTag AbilityTag;

	/**
	 * Retrieve ability definitions from currently equipped item slot.
	 */
	UPROPERTY(EditAnywhere, meta=(EditCondition="AbilitySource == ERecallAbilityTaskSource::Collection", EditConditionHides))
	bool bUseSelectedEquipSlot = false;

	UPROPERTY(EditAnywhere)
	FName Section = NAME_None;

	UPROPERTY(EditAnywhere)
	bool bIgnoreBlend = false;

	UPROPERTY(EditAnywhere)
	bool bSucceedOnFinish = true;
	
protected:
	TStateTreeExternalDataHandle<struct FRecallAbilityFragment> AbilityFragmentHandle;
	
	TStateTreeExternalDataHandle<struct FRecallEquipmentFragment, EStateTreeExternalDataRequirement::Optional> EquipmentFragmentHandle;

	TObjectPtr<URecallAbilityAsset> GetAbilityAsset(FStateTreeExecutionContext& Context) const;
};

USTRUCT()
struct RECALLABILITY_API FRecallAbilityChainTaskInstanceData
{
	GENERATED_BODY()

	/**
	 * Gameplay identifier for the sequential ability activation pattern.
	 */
	UPROPERTY(EditAnywhere, Category=Parameter, meta=(GameplayTagFilter="AbilityChain"))
	FGameplayTag AbilityChain;

	/**
	 * Directional vector influencing ability sequence selection logic.
	 */
	UPROPERTY(EditAnywhere, Category=Parameter)
	FVector2f Direction = FVector2f::ZeroVector;

	/**
	 * Entity references for coordinated ability sequence targeting.
	 */
	UPROPERTY(EditAnywhere, Category=Parameter)
	TArray<FMassEntityHandle> TargetEntities;
};

/**
 * State tree operation managing sequential ability execution patterns.
 * Handles chain initiation, continuation, and state tracking.
 */
USTRUCT(meta=(DisplayName="Ability Chain"))
struct RECALLABILITY_API FRecallAbilityChainTask : public FRecallStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FRecallAbilityChainTaskInstanceData;

	FRecallAbilityChainTask();

protected:
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	
	/**
	 * Determines the origin location for ability sequence definitions.
	 */
	UPROPERTY(EditAnywhere, DisplayName="Source")
	ERecallAbilityTaskSource AbilitySource = ERecallAbilityTaskSource::Collection;
	
	/**
	 * Direct reference to ability sequence configuration data.
	 */
	UPROPERTY(EditAnywhere, meta=(EditCondition="AbilitySource == ERecallAbilityTaskSource::Asset", EditConditionHides))
	TObjectPtr<URecallAbilityCollectionAsset> AbilityCollection;

	/**
	 * Source ability sequences from active equipment slot configuration.
	 */
	UPROPERTY(EditAnywhere, meta=(EditCondition="AbilitySource == ERecallAbilityTaskSource::Collection", EditConditionHides))
	bool bUseSelectedEquipSlot = false;
	
protected:
	TStateTreeExternalDataHandle<struct FRecallAbilityFragment> AbilityFragmentHandle;
	TStateTreeExternalDataHandle<struct FRecallAbilityChainFragment> AbilityChainFragmentHandle;
	TStateTreeExternalDataHandle<struct FRecallAbilityChainInputFragment> AbilityChainInputFragmentHandle;
	TStateTreeExternalDataHandle<struct FRecallEquipmentFragment, EStateTreeExternalDataRequirement::Optional> EquipmentFragmentHandle;

	TObjectPtr<const URecallAbilityCollectionAsset> GetAbilityCollectionAsset(FStateTreeExecutionContext& Context) const;
};
