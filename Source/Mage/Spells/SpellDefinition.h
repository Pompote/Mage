// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Elements/ElementTypes.h"
#include "Engine/DataAsset.h"
#include "SpellDefinition.generated.h"

class UGestureShapeTemplate;
class UTexture2D;
class USpellEffect;

// Portee de l'effet en jeu -- independant de la forme DESSINEE pour reconnaitre le sort (voir Shape
// ci-dessous, sans rapport). Voir README "Mecanique #2", table des sorts, colonne "Tageté/aoe".
UENUM(BlueprintType)
enum class EDeliveryShape : uint8
{
	Target	UMETA(DisplayName = "Target"),	// projectile mono-cible -- seule valeur implementee aujourd'hui
	AOE	UMETA(DisplayName = "AOE"),	// zone d'effet (mur, flaque...) -- pas encore implemente, voir Doc/claude-plan.md
	Cone	UMETA(DisplayName = "Cone"),	// cone (vent) -- pas encore implemente, voir Doc/claude-plan.md
};

// Capacite d'un sort a se propager vers des statuts "spreadable" a proximite -- voir README, colonne
// "spreadable" (Non / vent / Oui). Champ de donnees pret pour un futur systeme de vent/propagation
// (voir Doc/claude-plan.md) ; rien ne le lit encore.
UENUM(BlueprintType)
enum class ESpreadBehavior : uint8
{
	None	UMETA(DisplayName = "None"),
	WindOnly	UMETA(DisplayName = "Wind Only"),
	Any	UMETA(DisplayName = "Any"),
};

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

	// Identite stable de ce sort, independante du nom/chemin de l'asset -- meme principe que
	// UGestureShapeTemplate::ShapeId. Cle de recherche dans la table de reaction par sort (voir
	// Doc/claude-plan.md) ; laisser a None si ce sort n'a pas besoin de reactions specifiques (seule
	// la table par element s'appliquera alors).
	UPROPERTY(EditDefaultsOnly, Category = "Spell")
	FName SpellId;

	// Portee de l'effet en jeu -- voir README, colonne "Tageté/aoe". Seul Target est implemente
	// aujourd'hui (voir Doc/claude-plan.md) ; les deux autres valeurs existent pour etre choisies des
	// maintenant sans attendre le code qui les fera fonctionner.
	UPROPERTY(EditDefaultsOnly, Category = "Spell")
	EDeliveryShape DeliveryShape = EDeliveryShape::Target;

	// Ce sort applique-t-il son element a un ennemi touche ? -- voir README, colonne "Imprègne un
	// ennemi". Vrai par defaut (comportement actuel de tous les sorts elementaires). Lu par
	// ASpellProjectile::OnHit.
	UPROPERTY(EditDefaultsOnly, Category = "Spell")
	bool bImprintsEnemy = true;

	// Cet element peut-il persister sur n'importe quelle surface (pas seulement un ennemi) ? -- voir
	// README, colonne "Toutes surfaces". Lu par ASpellProjectile::OnHit (voir ASurfaceElementResidue).
	UPROPERTY(EditDefaultsOnly, Category = "Spell")
	bool bPersistsOnAnySurface = false;

	// Capacite de ce sort a se propager (vent, etc.) -- voir README, colonne "spreadable". Champ de
	// donnees seul pour l'instant, voir Doc/claude-plan.md.
	UPROPERTY(EditDefaultsOnly, Category = "Spell")
	ESpreadBehavior SpreadBehavior = ESpreadBehavior::None;
};
