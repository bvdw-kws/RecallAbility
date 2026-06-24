// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallAbilityChainProcessors.h"

#include "Data/Ability/RecallAbilityCollectionAsset.h"
#include "MassExecutionContext.h"
#include "RecallSignalSubsystem.h"
#include "Simulation/Ability/RecallAbilityChainFragments.h"
#include "Simulation/Ability/RecallAbilityChainSignalTypes.h"
#include "Simulation/Ability/RecallAbilityProcessorGroupTypes.h"
#include "Simulation/StateTree/RecallStateTreeProcessorGroupTypes.h"
#include "Simulation/StateTree/RecallStateTreeSignalTypes.h"
#include "Simulation/Transform/RecallTransformFragments.h"
#include "StateTree/RecallStateTreeExecutionContext.h"
#include "System/AI/RecallStateTreeSubsystem.h"
#include "System/Asset/RecallAssetManagerSubsystem.h"
#include "System/Random/RecallRandomNumberSubsystem.h"
#include "Utility/Simulation/RecallSimulationUtils.h"

//----------------------------------------------------------------------//
// URecallAbilityChainDestructor
//----------------------------------------------------------------------//
URecallAbilityChainDestructor::URecallAbilityChainDestructor()
	: EntityQuery(*this)
{
	ExecutionFlags = static_cast<int32>(EExtendedProcessorExecutionFlags::All);
	ObservedType = FRecallAbilityChainFragment::StaticStruct();
	Operation = EMassObservedOperation::Remove;
}

void URecallAbilityChainDestructor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager)
{
	Super::InitializeInternal(Owner, InEntityManager);
}

void URecallAbilityChainDestructor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FRecallAbilityChainFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddSubsystemRequirement<URecallStateTreeSubsystem>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddSubsystemRequirement<URecallAssetManagerSubsystem>(EMassFragmentAccess::ReadWrite);
}

void URecallAbilityChainDestructor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	QUICK_SCOPE_CYCLE_COUNTER(Recall_AbilityChain_Destructor);

	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		URecallStateTreeSubsystem& StateTreeSystem = Context.GetMutableSubsystemChecked<URecallStateTreeSubsystem>();
		URecallAssetManagerSubsystem& AssetManagerSystem = Context.GetMutableSubsystemChecked<URecallAssetManagerSubsystem>();
			
		const TArrayView<FRecallAbilityChainFragment> AbilityChangeList = Context.GetMutableFragmentView<FRecallAbilityChainFragment>();

		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); EntityIndex++)
		{
			FRecallAbilityChainFragment& AbilityChainFragment = AbilityChangeList[EntityIndex];
			StateTreeSystem.FreeInstanceData(AbilityChainFragment.ActiveAbilityChainInstance.AbilityChainHandle);

			for (TTuple<FSoftObjectPath, FRecallAssetLoadHandle>& AbilityChainAssetTuple : AbilityChainFragment.AbilityChainAssetMap)
			{
				AssetManagerSystem.ReleaseAsset(AbilityChainAssetTuple.Value);
			}

			AbilityChainFragment.AbilityChainAssetMap.Empty();
		}
	});
}

//----------------------------------------------------------------------//
// URecallAbilityChainInputProcessor
//----------------------------------------------------------------------//
URecallAbilityChainInputProcessor::URecallAbilityChainInputProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = static_cast<int32>(EExtendedProcessorExecutionFlags::All);
	ProcessingPhase = EMassProcessingPhase::PrePhysics;
	ExecutionOrder.ExecuteInGroup = Recall::Ability::ProcessorGroupNames::PrePhysics::AbilityChainInput;
	ExecutionOrder.ExecuteAfter.Add(Recall::StateTree::ProcessorGroupNames::StateTreeUpdate);
}

void URecallAbilityChainInputProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager)
{
	Super::InitializeInternal(Owner, InEntityManager);
}

void URecallAbilityChainInputProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	FMassTagBitSet RequiredTags;
	RequiredTags.Add(*FRecallAbilityChainActiveTag::StaticStruct());
	
	EntityQuery.AddRequirement<FRecallAbilityChainInputFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddTagRequirements<EMassFragmentPresence::All>(RequiredTags);
	EntityQuery.AddSubsystemRequirement<URecallSignalSubsystem>(EMassFragmentAccess::ReadWrite);
	EntityQuery.RegisterWithProcessor(*this);	
}

void URecallAbilityChainInputProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	QUICK_SCOPE_CYCLE_COUNTER(Recall_AbilityChainInput_Execute);

	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		URecallSignalSubsystem& SignalSystem = Context.GetMutableSubsystemChecked<URecallSignalSubsystem>();
		
		const TArrayView<FRecallAbilityChainInputFragment> InputList = Context.GetMutableFragmentView<FRecallAbilityChainInputFragment>();
		
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); EntityIndex++)
		{
			const FMassEntityHandle Entity = Context.GetEntity(EntityIndex);
			FRecallAbilityChainInputFragment& InputFragment = InputList[EntityIndex];

			const bool bWasAnyInputPressed = InputFragment.WasAnyInputPressed(RECALL_ABILITY_CHAIN_BUFFER_MAX_DURATION);
			
			uint16 PressedInputs = (InputFragment.Inputs ^ 0xFFFF) & InputFragment.InputBuffer;
			
			InputFragment.Inputs = InputFragment.InputBuffer;
			InputFragment.InputBuffer = 0;

			for (int32 InputIndex = 0; InputIndex < MAX_RECALL_ABILITY_CHAIN_INPUT_COUNT; InputIndex++)
			{
				const uint16 InputId = 1 << InputIndex;
				int32& Timer = InputFragment.PressedInputTimers[InputIndex];
				
				// Counting time duration after a button is pressed.
				if (Timer < RECALL_ABILITY_CHAIN_BUFFER_MAX_DURATION)
				{
					Timer++;
				}
				
				if ((PressedInputs & InputId) != 0)
				{
					Timer = 0;
				}
			}
			
			if (InputFragment.WasAnyInputPressed(RECALL_ABILITY_CHAIN_BUFFER_MAX_DURATION) || bWasAnyInputPressed)
			{
				SignalSystem.SignalEntity(Recall::AbilityChain::Signals::Input, Entity);
			}
		}
	});
}

//----------------------------------------------------------------------//
// URecallAbilityChainProcessor
//----------------------------------------------------------------------//
URecallAbilityChainProcessor::URecallAbilityChainProcessor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ExecutionFlags = static_cast<int32>(EExtendedProcessorExecutionFlags::All);
	ProcessingPhase = EMassProcessingPhase::PrePhysics;
	ExecutionOrder.ExecuteInGroup = Recall::Ability::ProcessorGroupNames::PrePhysics::AbilityChain;
	ExecutionOrder.ExecuteAfter.Add(Recall::Ability::ProcessorGroupNames::PrePhysics::AbilityChainInput);
}

void URecallAbilityChainProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager)
{
	Super::InitializeInternal(Owner, InEntityManager);
	
	SubscribeToSignal(Recall::AbilityChain::Signals::Start);
	SubscribeToSignal(Recall::AbilityChain::Signals::Input);
	SubscribeToSignal(Recall::AbilityChain::Signals::Delay);
	SubscribeToSignal(Recall::StateTree::Signals::TickRequired); // TODO: Use new signal?
}

void URecallAbilityChainProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FRecallAbilityChainFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FRecallAbilityChainInputFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddSubsystemRequirement<URecallStateTreeSubsystem>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddSubsystemRequirement<URecallSignalSubsystem>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddSubsystemRequirement<URecallAssetManagerSubsystem>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddSubsystemRequirement<URecallRandomNumberSubsystem>(EMassFragmentAccess::ReadWrite);
	EntityQuery.RegisterWithProcessor(*this);	
}

static void StopCurrentAbilityChain(FMassExecutionContext& Context,
	const FMassEntityHandle& Entity, FRecallAbilityChainFragment& AbilityChainFragment)
{
	FRecallAbilityChainInstance& AbilityChainInstance = AbilityChainFragment.ActiveAbilityChainInstance;
	if (!AbilityChainInstance.AbilityCollection)
	{
		return;
	}
	
	// Release the previous ability chain
	if (AbilityChainInstance.AbilityChainHandle.IsValid())
	{
		FMassEntityManager& EntityManager = Context.GetEntityManagerChecked();
		URecallAssetManagerSubsystem& AssetManagerSystem = Context.GetMutableSubsystemChecked<URecallAssetManagerSubsystem>();
		URecallStateTreeSubsystem& StateTreeSystem = Context.GetMutableSubsystemChecked<URecallStateTreeSubsystem>();
		URecallSignalSubsystem& SignalSystem = Context.GetMutableSubsystemChecked<URecallSignalSubsystem>();
		
		FStateTreeInstanceData* OldStateTreeInstanceData = StateTreeSystem.GetInstanceData(
			AbilityChainInstance.AbilityChainHandle);
		check(OldStateTreeInstanceData != nullptr);
		const FSoftObjectPath& OldAbilityChainAssetPath = AbilityChainInstance.AbilityCollection->AbilityChain;
		FRecallAssetLoadHandle& OldAbilityChainAssetHandle = AbilityChainFragment.AbilityChainAssetMap.FindChecked(
			OldAbilityChainAssetPath);
		UStateTree* OldStateTree = AssetManagerSystem.GetLoadedAsset<UStateTree>(OldAbilityChainAssetHandle);
		check(OldStateTree);
		
		FRecallStateTreeExecutionContext StateTreeContext(StateTreeSystem, *OldStateTree, *OldStateTreeInstanceData,
			EntityManager, SignalSystem, Context, Entity);
		StateTreeContext.Stop();

		StateTreeSystem.FreeInstanceData(AbilityChainInstance.AbilityChainHandle);
	}
				
	AbilityChainInstance.AbilityCollection = nullptr;
}

void URecallAbilityChainProcessor::SignalEntities(FMassEntityManager& EntityManager,
	FMassExecutionContext& Context, FRecallSignalNameLookup& EntitySignals)
{
	QUICK_SCOPE_CYCLE_COUNTER(Recall_AbilityChain_Signal);

	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		const double TimeInSeconds = Recall::Simulation::Utils::GetTimeSeconds(Context.GetWorld());

		FMassEntityManager& EntityManager = Context.GetEntityManagerChecked();
		
		URecallAssetManagerSubsystem& AssetManagerSystem = Context.GetMutableSubsystemChecked<URecallAssetManagerSubsystem>();
		URecallStateTreeSubsystem& StateTreeSystem = Context.GetMutableSubsystemChecked<URecallStateTreeSubsystem>();
		URecallSignalSubsystem& SignalSystem = Context.GetMutableSubsystemChecked<URecallSignalSubsystem>();
		URecallRandomNumberSubsystem& RandomNumberSystem = Context.GetMutableSubsystemChecked<URecallRandomNumberSubsystem>();

		const bool bIsActive = Context.DoesArchetypeHaveTag<FRecallAbilityChainActiveTag>();
		
		const TArrayView<FRecallAbilityChainFragment> AbilityChainList = Context.GetMutableFragmentView<FRecallAbilityChainFragment>();
		const TArrayView<FRecallAbilityChainInputFragment> InputList = Context.GetMutableFragmentView<FRecallAbilityChainInputFragment>();
		
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); EntityIndex++)
		{
			const FMassEntityHandle Entity = Context.GetEntity(EntityIndex);
			FRecallAbilityChainFragment& AbilityChainFragment = AbilityChainList[EntityIndex];
			FRecallAbilityChainInstance& AbilityChainInstance = AbilityChainFragment.ActiveAbilityChainInstance;
			FRecallAbilityChainInputFragment& InputFragment = InputList[EntityIndex];
			
			if (AbilityChainFragment.NextAbilityChainCollection)
			{
				StopCurrentAbilityChain(Context, Entity, AbilityChainFragment);

				// Start the new ability chain.
				const FSoftObjectPath& NextAbilityChainAssetPath = AbilityChainFragment.NextAbilityChainCollection->AbilityChain;
				check(!NextAbilityChainAssetPath.IsNull());
				FRecallAssetLoadHandle& NextAbilityChainAssetHandle = AbilityChainFragment.AbilityChainAssetMap.FindOrAdd(
					NextAbilityChainAssetPath);
				if (!NextAbilityChainAssetHandle.IsValid())
				{
					NextAbilityChainAssetHandle = AssetManagerSystem.RequestAsset(NextAbilityChainAssetPath);
				}
				
				AbilityChainInstance.AbilityCollection = AbilityChainFragment.NextAbilityChainCollection;
				AbilityChainFragment.NextAbilityChainCollection = nullptr;
			}
			
			if (!AbilityChainInstance.IsValid())
			{
				continue;
			}

			const FSoftObjectPath& AbilityChainAssetPath = AbilityChainInstance.AbilityCollection->AbilityChain;
			FRecallAssetLoadHandle& AbilityChainAssetHandle = AbilityChainFragment.AbilityChainAssetMap.FindChecked(
				AbilityChainAssetPath);
			if (!AssetManagerSystem.IsAssetLoaded(AbilityChainAssetHandle))
			{
				SignalSystem.DelaySignalEntity(Recall::AbilityChain::Signals::Start, Entity, 0.1f);
				continue;
			}

			// When the ability chain is activated, give one frame of delay so the input buffer can be processed.
			if (!bIsActive)
			{
				Context.Defer().AddTag<FRecallAbilityChainActiveTag>(Entity);
				continue;
			}
			
			bool bStart = false;
			
			if (!AbilityChainInstance.AbilityChainHandle.IsValid())
			{
				AbilityChainInstance.AbilityChainHandle = StateTreeSystem.AllocateInstanceData();
				bStart = true;
			}

			FStateTreeInstanceData* StateTreeInstanceData = StateTreeSystem.GetInstanceData(AbilityChainInstance.AbilityChainHandle);
			check(StateTreeInstanceData != nullptr);
			UStateTree* StateTree = AssetManagerSystem.GetLoadedAsset<UStateTree>(AbilityChainAssetHandle);
			
			FRecallStateTreeExecutionContext StateTreeContext(StateTreeSystem, *StateTree, *StateTreeInstanceData,
				EntityManager, SignalSystem, Context, Entity);

			if (bStart)
			{
				const int32 RandomSeed = RandomNumberSystem.GetRandomStream().RandRange(
					TNumericLimits<int32>::Min(), TNumericLimits<int32>::Max());
				StateTreeContext.Start(nullptr, RandomSeed);
				AbilityChainFragment.LastUpdateTimeInSeconds = TimeInSeconds;
			}
			else
			{
				// Compute adjusted delta time
				const float AdjustedDeltaTime = FloatCastChecked<float>(TimeInSeconds - AbilityChainFragment.LastUpdateTimeInSeconds, /* Precision */ 1. / 256.);
				AbilityChainFragment.LastUpdateTimeInSeconds = TimeInSeconds;
				
				const EStateTreeRunStatus RunStatus = StateTreeContext.Tick(AdjustedDeltaTime);
				if (RunStatus != EStateTreeRunStatus::Running)
				{
					StopCurrentAbilityChain(Context, Entity, AbilityChainFragment);
					InputFragment.ClearInputs();
					Context.Defer().RemoveTag<FRecallAbilityChainActiveTag>(Entity);
				}
			}
		}
	});
}

//----------------------------------------------------------------------//
// URecallAbilityChainDebugRepresentationProcessor
//----------------------------------------------------------------------//
#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
static bool bDebugRecallAbilityChainOnScreenLog = false;
static FAutoConsoleVariableRef CVarRecallAbilityChainOnScreenLog(
	TEXT("Recall.Ability.DebugAbilityChainOnScreen"),
	bDebugRecallAbilityChainOnScreenLog,
	TEXT("Display Ability Chain Log On Screen")
);

static bool bDebugRecallAbilityChainInWorldLog = false;
static FAutoConsoleVariableRef CVarRecallAbilityChainInWorldLog(
	TEXT("Recall.Ability.DebugAbilityChainInWorld"),
	bDebugRecallAbilityChainInWorldLog,
	TEXT("Display Ability Chain Log In World")
);
#endif // UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT

URecallAbilityChainDebugRepresentationProcessor::URecallAbilityChainDebugRepresentationProcessor()
	: EntityQuery(*this)
{
	ExecutionFlags = static_cast<int32>(EExtendedProcessorExecutionFlags::All);
	ProcessingPhase = EMassProcessingPhase::Render;
	bRequiresGameThreadExecution = true;
}

void URecallAbilityChainDebugRepresentationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	FMassTagBitSet RequiredTags;
	RequiredTags.Add(*FRecallAbilityChainActiveTag::StaticStruct());
	
	EntityQuery.AddRequirement<FRecallTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FRecallAbilityChainFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddTagRequirements<EMassFragmentPresence::All>(RequiredTags);
	EntityQuery.AddSubsystemRequirement<URecallStateTreeSubsystem>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddSubsystemRequirement<URecallSignalSubsystem>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddSubsystemRequirement<URecallAssetManagerSubsystem>(EMassFragmentAccess::ReadOnly);
#endif // UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
}

void URecallAbilityChainDebugRepresentationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	QUICK_SCOPE_CYCLE_COUNTER(Recall_AbilityChain_DebugRender);

	if (!bDebugRecallAbilityChainOnScreenLog && !bDebugRecallAbilityChainInWorldLog)
	{
		return;
	}

	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		FMassEntityManager& EntityManager = Context.GetEntityManagerChecked();
		const URecallAssetManagerSubsystem& AssetManagerSystem = Context.GetSubsystemChecked<URecallAssetManagerSubsystem>();
		URecallSignalSubsystem& SignalSystem = Context.GetMutableSubsystemChecked<URecallSignalSubsystem>();
		URecallStateTreeSubsystem& StateTreeSystem = Context.GetMutableSubsystemChecked<URecallStateTreeSubsystem>();

		const TConstArrayView<FRecallAbilityChainFragment> AbilityChainList = Context.GetFragmentView<FRecallAbilityChainFragment>();
		const TConstArrayView<FRecallTransformFragment> TransformList = Context.GetFragmentView<FRecallTransformFragment>();

		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); EntityIndex++)
		{
			const FRecallAbilityChainFragment& AbilityChainFragment = AbilityChainList[EntityIndex];
			const FRecallAbilityChainInstance& AbilityChainInstance = AbilityChainFragment.ActiveAbilityChainInstance;

			const FRecallTransformFragment& TransformFragment = TransformList[EntityIndex];
			
			FString Status;

			FStateTreeInstanceData* InstanceData = StateTreeSystem.GetInstanceData(AbilityChainInstance.AbilityChainHandle);
			if (InstanceData)
			{
				const FSoftObjectPath& AbilityChainAssetPath = AbilityChainInstance.AbilityCollection->AbilityChain;
				const FRecallAssetLoadHandle& AbilityChainAssetHandle = AbilityChainFragment.AbilityChainAssetMap.FindRef(
					AbilityChainAssetPath);
				
				UStateTree* StateTree = AssetManagerSystem.GetLoadedAsset<UStateTree>(AbilityChainAssetHandle);
			
				const FRecallStateTreeExecutionContext StateTreeContext(StateTreeSystem, *StateTree, *InstanceData, EntityManager, SignalSystem, Context, Context.GetEntity(EntityIndex));

				Status += FString::Printf(TEXT("  %s\n"), *StateTreeContext.GetStateTree()->GetName());

				Status += FString::Printf(TEXT("  %s\n"), *StateTreeContext.GetActiveStateName());

#if WITH_STATETREE_DEBUG
				Status += FString::Printf(TEXT("  %d\n"), StateTreeContext.GetStateChangeCount());
#endif // WITH_STATETREE_DEBUG
			}
			else
			{
				Status += TEXT("{red}<No AbilityChain instance>{white}\n");
			}

			if (bDebugRecallAbilityChainInWorldLog)
			{
				DrawDebugString(Context.GetWorld(), TransformFragment.Position, Status, nullptr, FColor::Green, -1);
			}

			if (bDebugRecallAbilityChainOnScreenLog)
			{
				GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Green, Status);
			}
		}
	});
#endif // UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
}
