// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Elements/ElementTypes.h"
#include "SpellCastContext.generated.h"

class AActor;
class USpellDefinition;

// Contexte transmis a USpellEffect::Execute au moment du cast -- regroupe dans un seul struct ce
// qu'un effet peut avoir besoin de lire. Un UFUNCTION BlueprintNativeEvent ne pouvant pas etre
// surcharge, passer par un struct extensible evite de recasser la signature d'Execute (et donc de
// devoir reconnecter tout override Blueprint existant) au prochain besoin de contexte de cast.
USTRUCT(BlueprintType)
struct FSpellCastContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Spell")
	TObjectPtr<AActor> Caster = nullptr;

	// Element porte par le sort lance -- None si le sort n'en a pas (comportement inchange)
	UPROPERTY(BlueprintReadOnly, Category = "Spell")
	EElement Element = EElement::None;

	UPROPERTY(BlueprintReadOnly, Category = "Spell")
	TObjectPtr<USpellDefinition> SpellDefinition = nullptr;
};
