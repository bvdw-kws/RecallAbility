// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"

#include "RecallProjectileTableRow.generated.h"

USTRUCT(BlueprintType)
struct RECALLPROJECTILEMODULE_API FRecallProjectileTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<class UMassEntityConfigAsset> Entity;
	
	UPROPERTY(EditAnywhere, meta=(Units=CentimetersPerSecond, ClampMin=0.01f))
	float Speed = 100.0f;

	UPROPERTY(EditAnywhere, meta=(Units=Seconds, ClampMin=0.0f))
	float LifeSpan = 3.0f;

	UPROPERTY(EditAnywhere)
	bool bHoming = false;
	
	UPROPERTY(EditAnywhere, meta=(EditCondition=bHoming, ClampMin=0.0f, ClampMax=10.0f))
	float HomingInterpSpeed = 2.0f;

	float GetSpeedPerFrame() const;
};
