// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Elements/ElementTypes.h"
#include "Engine/DataAsset.h"
#include "SpellReactionTable.generated.h"

// Meme forme que FElementReactionRow (voir ElementReactionTable.h), mais la cle est une paire de
// sorts (USpellDefinition::SpellId) plutot qu'une paire d'elements -- permet des reactions
// specifiques a un sort precis (ex: "Boule de feu + Eclair" != "Mur de feu + Eclair" alors que les
// deux sont Feu+Foudre). Voir README "Mecanique #2", table des combinaisons par sort, et
// Doc/claude-plan.md.
USTRUCT(BlueprintType)
struct FSpellReactionRow
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Reaction")
	FName SpellIdA;

	UPROPERTY(EditDefaultsOnly, Category = "Reaction")
	FName SpellIdB;

	// Purement cosmetique (logs/debug) -- ex: "FoudreEnflammee"
	UPROPERTY(EditDefaultsOnly, Category = "Reaction")
	FName ReactionId;

	UPROPERTY(EditDefaultsOnly, Category = "Reaction")
	TSubclassOf<AActor> ReactionEffectClass;

	// Statut laisse sur la cible apres la reaction -- None (par defaut) = efface completement.
	UPROPERTY(EditDefaultsOnly, Category = "Reaction")
	EElement ResultElement = EElement::None;

	// Multiplie la duree de ResultElement. Sans effet si ResultElement == None.
	UPROPERTY(EditDefaultsOnly, Category = "Reaction")
	float IntensityMultiplier = 1.f;

	// Si faux, cette ligne documente explicitement "aucune interaction" -- le statut de la cible
	// reste intact. Equivalent fonctionnel a l'absence de ligne, juste plus explicite pour l'auteur
	// (voir par ex. "Boule de terre + champ electrique -> aucune interaction" dans le README).
	UPROPERTY(EditDefaultsOnly, Category = "Reaction")
	bool bClearTargetStatus = true;
};

// Table de reaction sort x sort : verifiee EN PREMIER par UElementReactionSubsystem::TryReact, avant
// de retomber sur UElementReactionTable si aucune ligne specifique au sort ne correspond (voir
// README, 40 lignes visees). Asset unique hand-authored, meme precedent que UElementReactionTable --
// une seule table existera jamais, pas de decouverte automatique.
UCLASS(BlueprintType)
class MAGE_API USpellReactionTable : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Reaction")
	TArray<FSpellReactionRow> Reactions;

	// Lookup non ordonne : SpellA+SpellB == SpellB+SpellA. Retourne nullptr si aucune ligne ne
	// correspond, ou si l'un des deux FName est None (un cote sans identite de sort connue ne peut
	// jamais matcher une ligne specifique au sort -- seule la table par element pourra s'appliquer).
	const FSpellReactionRow* FindReaction(FName SpellIdA, FName SpellIdB) const;
};
