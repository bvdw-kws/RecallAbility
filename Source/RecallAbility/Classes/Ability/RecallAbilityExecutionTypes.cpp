// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallAbilityExecutionTypes.h"

#include "MassEntityView.h"
#include "MassExecutionContext.h"

//----------------------------------------------------------------------//
// FRecallAbilityExecutionContext
//----------------------------------------------------------------------//
UWorld* FRecallAbilityExecutionContext::GetWorld() const
{
	return MassExecutionContext.GetWorld();
}

FMassEntityView FRecallAbilityExecutionContext::GetEntityView() const
{
	return FMassEntityView(GetEntityManager(), Entity);
}

FMassEntityManager&  FRecallAbilityExecutionContext::GetEntityManager() const
{
	return MassExecutionContext.GetEntityManagerChecked();
}
