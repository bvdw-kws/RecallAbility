// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"

namespace Recall::Ability::ProcessorGroupNames
{

	namespace PrePhysics
	{		
		const FName AbilityChain				= FName(TEXT("RecallAbilityAbilityChain"));
		const FName AbilityChainInput			= FName(TEXT("RecallAbilityAbilityChainInput"));		
	} // namespace PrePhysics
	
	namespace StartPhysics
	{
		const FName Animation					= FName(TEXT("RecallAbilityAnimation"));
		const FName Ability						= FName(TEXT("RecallAbilityAbility"));
	} // namespace StartPhysics

	namespace EndPhysics
	{
		const FName AnimationSync				= FName(TEXT("RecallAbilityAnimationSync"));
	} // namespace EndPhysics

} // namespace Recall::Ability::ProcessorGroupNames
