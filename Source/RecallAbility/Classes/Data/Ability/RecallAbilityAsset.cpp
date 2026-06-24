// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallAbilityAsset.h"

int32 URecallAbilityAsset::GetAnimationSectionStartFrame(int32 SectionIndex) const
{
	if (AnimationSections.Num() > 0)
	{
		int32 StartFrame = 0;
		
		for (int32 CurrentIndex = 0; CurrentIndex < SectionIndex; CurrentIndex++)
		{
			StartFrame += AnimationSections[CurrentIndex].Duration;
		}

		return StartFrame;
	}
	
	return TNumericLimits<int32>::Max();
}

int32 URecallAbilityAsset::GetDuration() const
{
	int32 Result = 0;

	for (const FRecallAbilityAnimationSection& Section : AnimationSections)
	{
		Result += Section.Duration;

		if (Section.bLoop)
		{
			for (int32 LoopIndex = 1; LoopIndex < Section.LoopCount; LoopIndex++)
			{
				Result += Section.Duration;
			}
		}
	}

	return Result;
}

float URecallAbilityAsset::GetSectionStartFrame(float Frame,
	const FRecallAbilityAnimationSection*& OutSection) const
{
	if (AnimationSections.Num() == 0)
	{
		checkNoEntry();
		return 0.0f;
	}
	
	// When there are animation sections, calculate the animation offset based on the section elapsed time.
	float Accum = 0.0f;
	float SectionStartFrame = 0.0f;
	
	for (const FRecallAbilityAnimationSection& Section : AnimationSections)
	{
		OutSection = &Section;
		Accum += Section.Duration;

		if (Frame < Accum)
		{
			break;
		}

		SectionStartFrame += Section.Duration;
	}

	check(OutSection != nullptr);
	return SectionStartFrame;
}
