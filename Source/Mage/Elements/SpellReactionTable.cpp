// Fill out your copyright notice in the Description page of Project Settings.

#include "Elements/SpellReactionTable.h"

const FSpellReactionRow* USpellReactionTable::FindReaction(FName SpellIdA, FName SpellIdB) const
{
	if (SpellIdA.IsNone() || SpellIdB.IsNone())
	{
		return nullptr;
	}

	return Reactions.FindByPredicate([SpellIdA, SpellIdB](const FSpellReactionRow& Row)
	{
		return (Row.SpellIdA == SpellIdA && Row.SpellIdB == SpellIdB) || (Row.SpellIdA == SpellIdB && Row.SpellIdB == SpellIdA);
	});
}
