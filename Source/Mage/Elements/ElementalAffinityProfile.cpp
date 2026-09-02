// Fill out your copyright notice in the Description page of Project Settings.

#include "Elements/ElementalAffinityProfile.h"

FElementAffinityEntry UElementalAffinityProfile::GetAffinity(EElement Element) const
{
	if (const FElementAffinityEntry* Found = Affinities.FindByPredicate([Element](const FElementAffinityEntry& Entry)
	{
		return Entry.Element == Element;
	}))
	{
		return *Found;
	}

	FElementAffinityEntry Default;
	Default.Element = Element;
	return Default;
}
