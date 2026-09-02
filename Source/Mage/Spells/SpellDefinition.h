// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Elements/ElementTypes.h"
#include "Engine/DataAsset.h"
#include "SpellDefinition.generated.h"

class UGestureShapeTemplate;
class UTexture2D;
class USpellEffect;

// Un sort : la forme qui le declenche (une seule en Phase 1, plusieurs en Phase 2 composition),
// son icone pour le HUD, et l'effet qu'il declenche au cast. EffectClass vide = le cast se
// contente de retirer le sort du buffer, sans autre effet -- suffisant pour l'instant.
UCLASS(BlueprintType)
class MAGE_API USpellDefinition : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Spell")
	TObjectPtr<UGestureShapeTemplate> Shape;

	UPROPERTY(EditDefaultsOnly, Category = "Spell")
	TObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditDefaultsOnly, Category = "Spell")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, Category = "Spell")
	TSubclassOf<USpellEffect> EffectClass;

	// Element porte par ce sort -- None = aucun (comportement inchange, voir README "Mecanique #2 :
	// Elements et interactions")
	UPROPERTY(EditDefaultsOnly, Category = "Spell")
	EElement Element = EElement::None;
};
