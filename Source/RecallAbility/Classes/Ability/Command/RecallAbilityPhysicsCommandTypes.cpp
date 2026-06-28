// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallAbilityPhysicsCommandTypes.h"

#include "Ability/RecallAbilityExecutionTypes.h"
#include "Physics/JPRPhysicsLayerDataAsset.h"
#include "MassEntityView.h"
#include "Physics/RecallPhysicsObjects.h"
#include "Simulation/Ability/RecallAbilityFragments.h"
#include "Simulation/Physics/RecallPhysicsBodyFragment.h"
#include "System/Physics/RecallPhysicsSubsystem.h"
#include "Utility/Math/RecallMathUtils.h"

//----------------------------------------------------------------------//
// FRecallAbilityAddForceCommand
//----------------------------------------------------------------------//
void FRecallAbilityAddForceCommand::Execute(const FRecallAbilityExecutionContext& Context, ERecallAbilityExecutionEvent Event) const
{
	if (Event != ERecallAbilityExecutionEvent::OnTick || !FrameRange.Contains(Context.AbilityFragment.ElapsedFrames))
	{
		return;
	}
	
	URecallPhysicsSubsystem* PhysicsSystemPtr = UWorld::GetSubsystem<URecallPhysicsSubsystem>(Context.GetWorld());
	checkf(PhysicsSystemPtr, TEXT("Invalid physics system"));
	
	const FMassEntityView EntityView = Context.GetEntityView();
	const FJPRPhysicsBodyFragment* BodyFragmentPtr = EntityView.GetFragmentDataPtr<FJPRPhysicsBodyFragment>();
	const FJPRPhysicsBodyView Body = BodyFragmentPtr != nullptr ? PhysicsSystemPtr->GetMutableBody(BodyFragmentPtr->BodyHandle) : nullptr;
	if (!ensureAlwaysMsgf(Body.IsValid(), TEXT("Entity must have a physics body")))
	{
		return;
	}

	const FVector ForceVector = Force * ForceDirection.GetSafeNormal();

	Body.AddImpulse(ForceVector);
}

//----------------------------------------------------------------------//
// FRecallAbilityPhysicsLayerCommand
//----------------------------------------------------------------------//
void FRecallAbilityPhysicsLayerCommand::Execute(const FRecallAbilityExecutionContext& Context,
	ERecallAbilityExecutionEvent Event) const
{
	const auto* BodyFragmentPtr = Context.GetEntityView().GetFragmentDataPtr<FJPRPhysicsBodyFragment>();
	URecallPhysicsSubsystem* PhysicsSystemPtr = UWorld::GetSubsystem<URecallPhysicsSubsystem>(Context.GetWorld());
	check(PhysicsSystemPtr);
	const TWeakObjectPtr<const UJPRPhysicsLayerDataAsset> PhysicsLayer = PhysicsSystemPtr->GetPhysicsLayer();
	if (BodyFragmentPtr == nullptr || !PhysicsLayer.IsValid())
	{
		return;
	}

	const int32 LayerIndex = PhysicsLayer->GetLayerIndex(Layer);
	if (LayerIndex == INDEX_NONE)
	{
		return;
	}

	switch (Event)
	{
	case ERecallAbilityExecutionEvent::OnEnter:
		PhysicsSystemPtr->SetLayerOverride(
			BodyFragmentPtr->BodyHandle, static_cast<uint16>(LayerIndex));
		break;
		
	case ERecallAbilityExecutionEvent::OnExit:
		PhysicsSystemPtr->ClearLayerOverride(BodyFragmentPtr->BodyHandle);
		break;

	default:
		break;
	}
}
