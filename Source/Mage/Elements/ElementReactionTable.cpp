// Fill out your copyright notice in the Description page of Project Settings.

#include "Elements/ElementReactionTable.h"

const FElementReactionRow* UElementReactionTable::FindReaction(EElement A, EElement B) const
{
	return Reactions.FindByPredicate([A, B](const FElementReactionRow& Row)
	{
		return (Row.ElementA == A && Row.ElementB == B) || (Row.ElementA == B && Row.ElementB == A);
	});
}
