// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "StateTreeSchema.h"
#include "StateTreeExecutionTypes.h"

#include "RecallAbilityChainStateTreeSchema.generated.h"

/**
 * StateTree to customize an ability chain (combo).
 */
UCLASS(EditInlineNew, CollapseCategories, meta=(DisplayName="MS Ability Chain Behavior Base", CommonSchema))
class RECALLABILITY_API URecallAbilityChainStateTreeSchema : public UStateTreeSchema
{
	GENERATED_BODY()

public:
	URecallAbilityChainStateTreeSchema();

protected:
	virtual bool IsStructAllowed(const UScriptStruct* InScriptStruct) const override;
	virtual bool IsExternalItemAllowed(const UStruct& InStruct) const override;
	virtual TConstArrayView<FStateTreeExternalDataDesc> GetContextDataDescs() const override;

protected:
	template <typename T>
	static bool IsStructChildOf(const UScriptStruct* InScriptStruct, bool bExcluseBaseStruct = true)
	{
		if (bExcluseBaseStruct && T::StaticStruct() == InScriptStruct)
		{
			return false;
		}
		return InScriptStruct->IsChildOf(T::StaticStruct());
	}
	
	template <typename T>
	typename TEnableIf<!TIsDerivedFrom<typename T::DataType, UObject>::IsDerived, void>::Type LinkContextData(T& Handle, FName Name, FGuid Guid)
	{
		LinkContextData(Handle, Name, T::DataType::StaticStruct(), Guid);
	}

	void LinkContextData(FStateTreeExternalDataHandle& Handle, FName Name, const UStruct* Struct, FGuid Guid)
	{
		const FStateTreeExternalDataDesc Desc(Name, Struct, Guid);
		int32 Index = ContextDataDescs.Find(Desc);
		if (Index == INDEX_NONE)
		{
			Index = ContextDataDescs.Add(Desc);
			check(FStateTreeDataHandle::IsValidIndex(Index + ContextDataBaseIndex));
			ContextDataDescs[Index].Handle.DataHandle = FStateTreeDataHandle(EStateTreeDataSourceType::ContextData, Index + ContextDataBaseIndex);
		}
		Handle.DataHandle = FStateTreeDataHandle(EStateTreeDataSourceType::ContextData, Index + ContextDataBaseIndex);
	}

private:
	UPROPERTY()
	TArray<FStateTreeExternalDataDesc> ContextDataDescs;
	UPROPERTY()
	int32 ContextDataBaseIndex = 0;

	TStateTreeExternalDataHandle<struct FRecallAbilityChainFragment> AbilityChainFragment;
	TStateTreeExternalDataHandle<struct FRecallAbilityChainInputFragment> AbilityChainInputFragment;
};
