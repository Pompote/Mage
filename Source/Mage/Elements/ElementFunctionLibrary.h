// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Elements/ElementTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ElementFunctionLibrary.generated.h"

// Point d'entree partage pour "livrer un element au contact" -- utilise par ASpellProjectile::OnHit
// aujourd'hui, et par le futur jet (grab/lancer) plus tard. Centralise cette logique en un seul
// endroit plutot que de la dupliquer par type de porteur (voir README "Mecanique #2").
UCLASS()
class MAGE_API UElementFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// SpellId identifie le sort qui delivre Element (voir USpellDefinition::SpellId) -- None si
	// l'appelant n'a pas d'identite de sort a donner, auquel cas seule la table de reaction par
	// element pourra s'appliquer en cas de reaction (voir Doc/claude-plan.md). Retourne false si
	// Element == None, TargetActor est null, ou TargetActor n'a pas de UElementalStatusComponent (ce
	// dernier cas n'est pas une erreur -- une cible non-elementaire est juste un non-evenement)
	UFUNCTION(BlueprintCallable, Category = "Element")
	static bool DeliverElementToActor(EElement Element, FName SpellId, AActor* TargetActor, AActor* SourceActor, FVector ContactLocation);
};
