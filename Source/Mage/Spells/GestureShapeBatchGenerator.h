// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UObject/SoftObjectPath.h"
#include "GestureShapeBatchGenerator.generated.h"

// Outil editor-only : cree et bake automatiquement un UGestureShapeTemplate pour chaque image du
// dossier source qui n'en a pas deja un. Evite de creer les assets un par un a la main, tout en
// gardant un fichier separe par forme (compatible Asset Manager, pas de conflit git entre deux
// contributeurs qui ajoutent des formes differentes en parallele).
//
// A creer une seule fois (comme n'importe quel Data Asset), reutilisable ensuite a chaque nouvel
// ajout d'image : deposer l'image dans SourceImagesDirectory, cliquer le bouton, c'est tout.
UCLASS(BlueprintType)
class MAGE_API UGestureShapeBatchGenerator : public UDataAsset
{
	GENERATED_BODY()

public:
#if WITH_EDITORONLY_DATA
	// Dossier scanne pour trouver les images sources (Texture2D)
	UPROPERTY(EditAnywhere, Category = "Gesture Generator", meta = (RelativeToGameContentDir, LongPackageName))
	FDirectoryPath SourceImagesDirectory;

	// Dossier ou creer les nouveaux UGestureShapeTemplate
	UPROPERTY(EditAnywhere, Category = "Gesture Generator", meta = (RelativeToGameContentDir, LongPackageName))
	FDirectoryPath TargetDirectory;
#endif

#if WITH_EDITOR
	// Pour chaque image de SourceImagesDirectory pas encore associee a un UGestureShapeTemplate
	// existant dans TargetDirectory : cree l'asset, assigne l'image, deduit le ShapeId du nom de
	// fichier, et lance le bake immediatement. Rejouable sans risque -- les images deja utilisees
	// sont detectees et ignorees.
	UFUNCTION(CallInEditor, Category = "Gesture Generator")
	void GenerateMissingShapeTemplates();
#endif
};
