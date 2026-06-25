// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "MassEntityTraitBase.h"

#include "RecallAbilityTraits.generated.h"

/**
* Trait for entities that can play ability assets.
*/
UCLASS(meta=(DisplayName="RE Ability"))
class RECALLABILITY_API URecallAbilityTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

public:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;

protected:
	/**
	 * Optional collection of abilities so an ability can be triggered by tag.
	 */
	UPROPERTY(EditAnywhere)
	TObjectPtr<URecallAbilityCollectionAsset> AbilityCollection;

};
