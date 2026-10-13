// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Elements/ElementTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ElementReactionSubsystem.generated.h"

class UElementReactionTable;
class USpellReactionTable;

// Point d'entree unique pour resoudre une reaction elementaire -- meme precedent que
// USpellRecognitionSubsystem (charge une fois par session, joignable depuis n'importe ou via
// GetGameInstance()->GetSubsystem<>()).
UCLASS(config = Game)
class MAGE_API UElementReactionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// Cherche d'abord une reaction specifique a la paire de sorts (SpellIdA/SpellIdB) dans
	// SpellReactionTable, puis retombe sur la paire d'elements (ElementA/ElementB) dans
	// ReactionTable si aucune ligne specifique au sort n'existe (voir Doc/claude-plan.md). Si
	// trouvee : log, spawn ReactionEffectClass a Location (avec Setup(TargetActor, SourceActor,
	// ElementA, ElementB) si c'est un AElementReactionEffectActor), notifie SourceActor via
	// IElementReactionCarrier s'il l'implemente, et remplit les parametres de sortie decrivant
	// l'etat a appliquer sur la cible. Retourne false si aucune ligne ne correspond dans aucune des
	// deux tables.
	UFUNCTION(BlueprintCallable, Category = "Element")
	bool TryReact(FName SpellIdA, EElement ElementA, FName SpellIdB, EElement ElementB,
		AActor* TargetActor, AActor* SourceActor, FVector Location,
		EElement& OutResultElement, float& OutIntensityMultiplier, bool& OutClearTargetStatus);

private:
	// Chemin de la table a charger au demarrage -- voir Config/DefaultGame.ini, section
	// [/Script/Mage.ElementReactionSubsystem], meme esprit que AssetManagerSettings pour les formes
	UPROPERTY(config)
	FSoftObjectPath ReactionTableAssetPath;

	// Optionnelle -- si aucun asset n'est configure (ou pas encore cree, voir Doc/claude-plan.md),
	// TryReact retombe silencieusement sur ReactionTable seule, comportement identique a aujourd'hui.
	UPROPERTY(config)
	FSoftObjectPath SpellReactionTableAssetPath;

	UPROPERTY()
	TObjectPtr<UElementReactionTable> ReactionTable;

	UPROPERTY()
	TObjectPtr<USpellReactionTable> SpellReactionTable;
};
