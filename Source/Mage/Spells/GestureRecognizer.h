// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GestureRecognizer.generated.h"

// Un template de forme deja normalise (reechantillonne, tourne, mis a l'echelle, translate a
// l'origine) par l'implementation du recognizer qui l'a produit -- voir IGestureRecognizer::Normalize.
USTRUCT(BlueprintType)
struct FGestureTemplate
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gesture")
	FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gesture")
	TArray<FVector2D> NormalizedPoints;
};

USTRUCT(BlueprintType)
struct FGestureRecognitionResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Gesture")
	FName TemplateId = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Gesture")
	float Score = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Gesture")
	bool bMatched = false;
};

// Interface abstraite pour un algorithme de reconnaissance de gestes/formes.
// Permet de remplacer l'implementation ($1 aujourd'hui, $N multi-trait plus tard pour la Phase 2)
// sans que le code appelant (capture, bibliotheque de formes, outil de bake) n'ait a changer.
class IGestureRecognizer
{
public:
	virtual ~IGestureRecognizer() = default;

	// Normalise un trace brut (reechantillonnage, rotation, mise a l'echelle, translation).
	// A appeler une fois par template au moment du bake, plutot qu'a chaque comparaison.
	virtual TArray<FVector2D> Normalize(const TArray<FVector2D>& RawPoints) const = 0;

	// RawStroke est normalise en interne ; Templates sont supposes deja normalises.
	virtual FGestureRecognitionResult Recognize(const TArray<FVector2D>& RawStroke, const TArray<FGestureTemplate>& Templates) const = 0;
};
