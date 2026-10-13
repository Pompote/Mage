// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Elements/ElementTypes.h"
#include "Engine/DataAsset.h"
#include "ElementReactionTable.generated.h"

USTRUCT(BlueprintType)
struct FElementReactionRow
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Reaction")
	EElement ElementA = EElement::None;

	UPROPERTY(EditDefaultsOnly, Category = "Reaction")
	EElement ElementB = EElement::None;

	// Purement cosmetique (logs/debug) en v1 -- ex: "Brume", "Boue", "Roche incandescente"
	UPROPERTY(EditDefaultsOnly, Category = "Reaction")
	FName ReactionId;

	// v1 : toutes les lignes peuvent pointer vers le meme Blueprint placeholder partage -- rien
	// n'empeche d'assigner une classe differente par ligne plus tard pour du gameplay distinct,
	// sans toucher au code de resolution ni a cette structure.
	UPROPERTY(EditDefaultsOnly, Category = "Reaction")
	TSubclassOf<AActor> ReactionEffectClass;

	// Statut laisse sur la cible apres la reaction -- None (par defaut) = efface completement
	// (comportement actuel, inchange). Une valeur differente decrit un resultat compose (ex: "eau
	// electrifiee" = Water + IntensityMultiplier) sans ajouter un nouvel etat a EElement. Voir
	// Doc/claude-plan.md.
	UPROPERTY(EditDefaultsOnly, Category = "Reaction")
	EElement ResultElement = EElement::None;

	// Multiplie la duree de ResultElement (couvre par ex. un statut "ameliore" sans champ dedie).
	// Sans effet si ResultElement == None.
	UPROPERTY(EditDefaultsOnly, Category = "Reaction")
	float IntensityMultiplier = 1.f;

	// Si faux, cette ligne documente explicitement "aucune interaction" -- le statut de la cible
	// reste intact. Equivalent fonctionnel a l'absence de ligne, juste plus explicite pour l'auteur.
	UPROPERTY(EditDefaultsOnly, Category = "Reaction")
	bool bClearTargetStatus = true;
};

// Table de reaction : exactement 6 lignes pour {Fire,Water,Earth,Air} (voir README "Mecanique #2 :
// Elements et interactions"). Asset unique hand-authored -- une seule table existera jamais, pas
// de decouverte automatique comme pour les formes de sorts.
UCLASS(BlueprintType)
class MAGE_API UElementReactionTable : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Reaction")
	TArray<FElementReactionRow> Reactions;

	// Lookup non ordonne : Fire+Water == Water+Fire. Retourne nullptr si aucune ligne ne correspond.
	const FElementReactionRow* FindReaction(EElement A, EElement B) const;
};
