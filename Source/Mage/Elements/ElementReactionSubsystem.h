// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Elements/ElementTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ElementReactionSubsystem.generated.h"

class UElementReactionTable;

// Point d'entree unique pour resoudre une reaction elementaire -- meme precedent que
// USpellRecognitionSubsystem (charge une fois par session, joignable depuis n'importe ou via
// GetGameInstance()->GetSubsystem<>()).
UCLASS(config = Game)
class MAGE_API UElementReactionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// Cherche la reaction pour (A,B) ; si trouvee, log + spawn ReactionEffectClass a Location.
	// Retourne false si aucune ligne ne correspond ou si la table n'est pas chargee.
	UFUNCTION(BlueprintCallable, Category = "Element")
	bool TryReact(EElement A, EElement B, FVector Location, AActor* SourceActor);

private:
	// Chemin de la table a charger au demarrage -- voir Config/DefaultGame.ini, section
	// [/Script/Mage.ElementReactionSubsystem], meme esprit que AssetManagerSettings pour les formes
	UPROPERTY(config)
	FSoftObjectPath ReactionTableAssetPath;

	UPROPERTY()
	TObjectPtr<UElementReactionTable> ReactionTable;
};
