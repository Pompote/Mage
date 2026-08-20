// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Spells/GestureRecognizer.h"
#include "SpellRecognitionSubsystem.generated.h"

class UDollarOneRecognizer;

// Point d'entree unique pour la reconnaissance de formes, charge une fois par session de jeu.
// Decouvre tous les UGestureShapeTemplate via l'Asset Manager (voir Config/DefaultGame.ini,
// section AssetManagerSettings, dossier /Game/Spells/Shapes) et les expose au format attendu
// par IGestureRecognizer -- la capture (USpellDrawComponent) n'a besoin de connaitre que ce Subsystem,
// jamais les Data Assets ni comment les formes sont chargees.
UCLASS()
class MAGE_API USpellRecognitionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Spells")
	FGestureRecognitionResult RecognizeStroke(const TArray<FVector2D>& RawStroke) const;

	int32 GetLoadedTemplateCount() const { return LoadedTemplates.Num(); }

private:
	UPROPERTY()
	TObjectPtr<UDollarOneRecognizer> Recognizer;

	TArray<FGestureTemplate> LoadedTemplates;

	void LoadShapeTemplates();
};
