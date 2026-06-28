// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "StateTree/RecallAbilityChainStateTreeTaskBase.h"

#include "RecallAbilityChainTasks.generated.h"

USTRUCT()
struct RECALLABILITY_API FRecallAbilityChainExecTaskInstanceData
{
	GENERATED_BODY()

	/**
	 * Return true when the ability can be canceled.
	 */
	UPROPERTY(VisibleAnywhere, Category=Output)
	bool bCanExit = false;

	/**
	 * Cache the name of the active task when this task was entered.
	 */
	UPROPERTY()
	FName StateName = NAME_None;
};

/**
 * Task to execute an ability from an ability chain.
 */
USTRUCT(meta=(DisplayName="Ability"))
struct RECALLABILITY_API FRecallAbilityChainExecTask :
	public FRecallAbilityChainStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FRecallAbilityChainExecTaskInstanceData;

	FRecallAbilityChainExecTask();
	
protected:
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

	/**
	 * Succeed in the task once the ability finished.
	 */
	UPROPERTY(EditAnywhere, Category=Parameter)
	bool bExitOnFinish = true;

	/**
	 * Tag of the ability to use from the ability collection set for the active ability chain.
	 */
	UPROPERTY(EditAnywhere, Category=Parameter, meta=(GameplayTagFilter="Ability"))
	FGameplayTag AbilityTag;

	/**
	 * Section of the ability to enter when the ability is executed.
	 */
	UPROPERTY(EditAnywhere, Category=Parameter)
	FName Section = NAME_None;

	/**
	 * Whether to skip the animation blend when entering the first ability section.
	 */
	UPROPERTY(EditAnywhere, Category=Parameter)
	bool bIgnoreBlend = false;
	
protected:
	TStateTreeExternalDataHandle<struct FRecallAbilityFragment> AbilityFragmentHandle;
	TStateTreeExternalDataHandle<struct FRecallAbilityChainFragment> AbilityChainFragmentHandle;
	TStateTreeExternalDataHandle<struct FRecallAbilityChainInputFragment> AbilityChainInputFragmentHandle;
	
	TObjectPtr<URecallAbilityAsset> GetAbilityAsset(FStateTreeExecutionContext& Context) const;
};

USTRUCT()
struct RECALLABILITY_API FRecallAbilityChainDelayTaskInstanceData
{
	GENERATED_BODY()

	/** Internal countdown in seconds. */
	UPROPERTY()
	float RemainingTime = 0.f;
};

/**
 * Simple task to wait indefinitely or for a given time (in seconds) before succeeding.
 */
USTRUCT(meta=(DisplayName="Delay Task"))
struct RECALLABILITY_API FRecallAbilityChainDelayTask
	: public FRecallAbilityChainStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FRecallAbilityChainDelayTaskInstanceData;

public:
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

	/** Delay before the task ends. */
	UPROPERTY(EditAnywhere, meta=(EditCondition="!bRunForever", Units="Seconds", ClampMin="0.0"))
	float Duration = 1.f;

	/** Adds random range to the Duration. */
	UPROPERTY(EditAnywhere, meta=(EditCondition="!bRunForever", Units="Seconds", ClampMin="0.0"))
	float RandomDeviation = 0.f;

	/** If true the task will run forever until a transition stops it. */
	UPROPERTY(EditAnywhere)
	bool bRunForever = false;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
	virtual FName GetIconName() const override
	{
		return FName("StateTreeEditorStyle|Node.Time");
	}
	virtual FColor GetIconColor() const override
	{
		return UE::StateTree::Colors::Grey;
	}
#endif // WITH_EDITOR
};

USTRUCT()
struct RECALLABILITY_API FRecallAbilityChainRotateTaskInstanceData
{
	GENERATED_BODY()

	/**
	 * Rotation applied to the owner entity when this task is active.
	 */
	UPROPERTY(EditAnywhere, Category=Parameter)
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY()
	FRotator EnterRotation = FRotator::ZeroRotator;
};

/**
 * Rotate the character when this task is active.
 */
USTRUCT(meta=(DisplayName="Rotate"))
struct RECALLABILITY_API FRecallAbilityChainRotateTask :
	public FRecallAbilityChainStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FRecallAbilityChainRotateTaskInstanceData;

	FRecallAbilityChainRotateTask();
	
protected:
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

	/**
	 * Remove the rotation when the task is exited.
	 */
	UPROPERTY(EditAnywhere, Category=Parameter)
	bool bRestoreRotationOnExit = true;
	
protected:
	TStateTreeExternalDataHandle<struct FJPRPhysicsBodyFragment> BodyFragmentHandle;
	TStateTreeExternalDataHandle<class URecallPhysicsSubsystem> PhysicsSystemHandle;
};