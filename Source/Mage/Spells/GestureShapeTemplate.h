// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Spells/GestureRecognizer.h"
#include "GestureShapeTemplate.generated.h"

class UTexture2D;

// Une forme reconnaissable, editable par un designer/artiste : image source (authoring + icone UI),
// points normalises (baked une fois via l'outil de bake, jamais calcules au runtime), et ID pour le
// lookup forme -> sort fait par la couche de resolution (plus tard).
//
// UPrimaryDataAsset plutot que UDataAsset : permet la decouverte automatique par l'Asset Manager
// une fois configuree (etape suivante du plan), sans avoir a migrer la classe a ce moment-la.
UCLASS(BlueprintType)
class MAGE_API UGestureShapeTemplate : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	// Identifiant de la forme (utilise par la couche de resolution des sorts)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gesture")
	FName ShapeId;

	// Image source ; sert a la fois de reference pour le bake et d'icone UI (menu de sorts)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gesture")
	TObjectPtr<UTexture2D> SourceImage;

	// Points deja normalises (reechantillonnage/rotation/echelle/translation), remplis par l'outil
	// de bake -- jamais calcules au runtime a partir de l'image.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gesture")
	TArray<FVector2D> NormalizedPoints;

	// Seuil de reconnaissance propre a cette forme ; si <= 0, le seuil global du recognizer s'applique
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gesture")
	float RecognitionThresholdOverride = -1.f;

	// Vrai si l'outil de bake a deja rempli les points normalises
	bool IsBaked() const { return NormalizedPoints.Num() > 0; }

	// Convertit vers la structure minimale attendue par IGestureRecognizer::Recognize
	FGestureTemplate ToGestureTemplate() const;
};
