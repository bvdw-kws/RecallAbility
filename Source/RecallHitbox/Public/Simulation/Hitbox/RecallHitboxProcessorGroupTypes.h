// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"

namespace Recall::Hitbox::ProcessorGroupNames
{
	
	namespace PrePhysics
	{		
		/**
		 * Group during which queued attacks are reset so they do not carry over from the previous frame.
		 */
		const FName ResetAttack					= FName(TEXT("RecallResetAttack"));
	} // namespace PrePhysics
	
	namespace PostPhysics
	{		
		/**
		 * Group during which attacks are tested against vulnerable entities.
		 */
		const FName Hitbox						= FName(TEXT("RecallHitbox"));

		/**
		 * Group during which attack hits are processed.
		 */
		const FName Hit							= FName(TEXT("RecallAttackHit"));
	} // namespace PostPhysics
	
} // namespace Recall::Hitbox::ProcessorGroupNames
