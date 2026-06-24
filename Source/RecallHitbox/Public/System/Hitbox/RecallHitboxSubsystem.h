// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "System/Interface/RecallSimulationReactSystemInterface.h"

#include "RecallHitboxSubsystem.generated.h"

struct FMassEntityHandle;
struct FRecallAttackDefinition;
struct FRecallAttackQueue;

UCLASS()
class RECALLHITBOX_API URecallHitboxSubsystem : 
	public UWorldSubsystem,
	public IRecallSimulationReactSystemInterface
{
	GENERATED_BODY()

public:
	FRecallAttackQueue& GetMutableAttackQueue();
	const FRecallAttackQueue& GetAttackQueue() const;

	void PushAttack(const FMassEntityHandle& Instigator, const FTransform& Transform,
		const FRecallAttackDefinition& Attack, uint8 AttackLayers,
		const TArray<FMassEntityHandle>& Targets = {});
	
protected:
	// UWorldSubsystem implementation Begin
	virtual void Initialize(FSubsystemCollectionBase& Collection) override final;
	virtual void Deinitialize() override final;
	// UWorldSubsystem implementation End
	
	// IRecallSimulationReactSystemInterface implementation Begin
	virtual void Reset() override final;
	virtual void Save(const FRecallSnapshotContext& Context, FInstancedStruct& OutSnapshot) override final;
	virtual void Restore(const FRecallSnapshotContext& Context, const FInstancedStruct& InSnapshot) override final;
	// IRecallSimulationReactSystemInterface implementation End

private:
	TSharedPtr<FRecallAttackQueue> AttackQueue;

};

template<>
struct TMassExternalSubsystemTraits<URecallHitboxSubsystem> final
{
	enum
	{
		GameThreadOnly = false
	};
};
