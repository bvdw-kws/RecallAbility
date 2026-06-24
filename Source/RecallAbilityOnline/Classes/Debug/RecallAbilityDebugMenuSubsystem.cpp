// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallAbilityDebugMenuSubsystem.h"

#include "Debug/DebugMenuInterface.h"
#include "System/Debug/DebugMenuSubsystem.h"

void URecallAbilityDebugMenuSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Collection.InitializeDependency<UDebugMenuSubsystem>();
	
#if WITH_DEBUG_MENU
	DebugMenuSubsystem = UGameInstance::GetSubsystem<UDebugMenuSubsystem>(GetGameInstance());
	if (DebugMenuSubsystem.IsValid())
	{
		CreateDebugMenuItems(DebugMenuSubsystem->GetMutableDebugMenu());
	}
#endif // WITH_DEBUG_MENU
}

void URecallAbilityDebugMenuSubsystem::Deinitialize()
{
	Super::Deinitialize();
	
#if WITH_DEBUG_MENU
	DebugMenuSubsystem.Reset();
#endif // WITH_DEBUG_MENU
}

void URecallAbilityDebugMenuSubsystem::Tick(float DeltaTime)
{
}

TStatId URecallAbilityDebugMenuSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(URecallAbilityDebugMenuContent, STATGROUP_Tickables);
}

void URecallAbilityDebugMenuSubsystem::CreateDebugMenuItems(IDebugMenu& DebugMenu)
{
#if WITH_DEBUG_MENU
	// Ability
	{
		DebugMenu.AddItem_Bool(TEXT("Ability"), "Show Ability", false, TEXT("Recall.Ability.ShowAll"));
		DebugMenu.AddItem_Bool(TEXT("Ability"), "Show Ability Chain On Screen", false, TEXT("Recall.Ability.DebugAbilityChainOnScreen"));
		DebugMenu.AddItem_Bool(TEXT("Ability"), "Show Ability Chain In World", false, TEXT("Recall.Ability.DebugAbilityChainInWorld"));
	}
	
	// Hitbox
	{
		DebugMenu.AddItem_Bool(TEXT("Hitbox"), "Show Hitboxes", false, TEXT("Recall.Hitbox.ShowAll"));
	}
#endif // WITH_DEBUG_MENU
}
