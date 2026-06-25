// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "MassEntityTraitBase.h"
#include "Data/Projectile/RecallProjectileStateTypes.h"

#include "RecallProjectileTraits.generated.h"

UCLASS(meta=(DisplayName="RE Projectile"))
class RECALLPROJECTILEMODULE_API URecallProjectileTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

public:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;

protected:	
	/**
	 * Starting state of the projectile.
	 */
	UPROPERTY(EditAnywhere)
	FName DefaultState = TEXT("Default");

	UPROPERTY(EditAnywhere, meta=(ShowOnlyInnerProperties))
	FRecallProjectileStateCollection StateCollection;
};
