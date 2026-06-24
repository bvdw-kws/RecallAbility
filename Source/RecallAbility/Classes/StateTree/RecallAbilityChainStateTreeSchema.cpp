// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallAbilityChainStateTreeSchema.h"

#include "MassEntityElementTypes.h"
#include "RecallAbilityChainStateTreeConditionBase.h"
#include "RecallAbilityChainStateTreeEvaluatorBase.h"
#include "RecallAbilityChainStateTreeTaskBase.h"
#include "Subsystems/WorldSubsystem.h"

URecallAbilityChainStateTreeSchema::URecallAbilityChainStateTreeSchema()
	: Super()
{
	LinkContextData(AbilityChainFragment,		FName("Ability Chain Data"),		FGuid(TEXT("91080F2A-4C58-C15A-E306-EE9A81165B72")));
	LinkContextData(AbilityChainInputFragment,	FName("Ability Chain Input Data"),	FGuid(TEXT("CA0EF3E2-F197-33C6-98B5-47526F42FF4A")));
}

bool URecallAbilityChainStateTreeSchema::IsStructAllowed(const UScriptStruct* InScriptStruct) const
{
	// Only allow Recall evals and tasks,and common conditions.
	return IsStructChildOf<FStateTreeConditionCommonBase>(InScriptStruct)
		|| IsStructChildOf<FRecallAbilityChainStateTreeTaskBase>(InScriptStruct)
		|| IsStructChildOf<FRecallAbilityChainStateTreeEvaluatorBase>(InScriptStruct)
		|| IsStructChildOf<FRecallAbilityChainStateTreeConditionBase>(InScriptStruct);
}

bool URecallAbilityChainStateTreeSchema::IsExternalItemAllowed(const UStruct& InStruct) const
{
	// Allow only WorldSubsystems and fragments as external data.
	return InStruct.IsChildOf(UWorldSubsystem::StaticClass())
		|| InStruct.IsChildOf(FMassFragment::StaticStruct())
		|| InStruct.IsChildOf(FMassSharedFragment::StaticStruct())
		|| InStruct.IsChildOf(FMassConstSharedFragment::StaticStruct());
}

TConstArrayView<FStateTreeExternalDataDesc> URecallAbilityChainStateTreeSchema::GetContextDataDescs() const
{
	return ContextDataDescs;
}
