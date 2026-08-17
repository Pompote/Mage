// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Spells/GestureRecognizer.h"
#include "DollarOneRecognizer.generated.h"

// Implementation du $1 Unistroke Recognizer (Wobbrock, Wilson, Li - 2007) : reechantillonnage,
// rotation a l'angle indicatif, mise a l'echelle, translation a l'origine, puis recherche du
// meilleur angle de comparaison par section doree contre chaque template.
UCLASS(BlueprintType)
class MAGE_API UDollarOneRecognizer : public UObject, public IGestureRecognizer
{
	GENERATED_BODY()

public:
	virtual TArray<FVector2D> Normalize(const TArray<FVector2D>& RawPoints) const override;
	virtual FGestureRecognitionResult Recognize(const TArray<FVector2D>& RawStroke, const TArray<FGestureTemplate>& Templates) const override;

protected:
	// Nombre de points apres reechantillonnage
	UPROPERTY(EditAnywhere, Category = "Dollar One")
	int32 NumResamplePoints = 64;

	// Taille du carre de reference pour la mise a l'echelle
	UPROPERTY(EditAnywhere, Category = "Dollar One")
	float SquareSize = 250.f;

	// Plage de recherche d'angle (+/-) en degres pour la comparaison
	UPROPERTY(EditAnywhere, Category = "Dollar One")
	float ThetaRangeDegrees = 45.f;

	// Precision de la recherche d'angle en degres
	UPROPERTY(EditAnywhere, Category = "Dollar One")
	float ThetaDeltaDegrees = 2.f;

	// Score minimum (0-1) pour considerer une forme comme reconnue
	UPROPERTY(EditAnywhere, Category = "Dollar One")
	float RecognitionThreshold = 0.7f;

private:
	static double PathLength(const TArray<FVector2D>& Points);
	static TArray<FVector2D> Resample(const TArray<FVector2D>& Points, int32 N);

	// $1 n'est invariant qu'a la rotation/echelle, pas au sens de parcours (horaire/anti-horaire) --
	// un meme cercle trace dans les deux sens produirait des tres mauvais scores l'un contre l'autre
	// sans ca. Force systematiquement le sens anti-horaire (aire signee du polygone, formule du
	// lacet) pour que le sens de tracé du joueur n'ait plus d'importance.
	static TArray<FVector2D> CanonicalizeWindingOrder(const TArray<FVector2D>& Points);

	static FVector2D Centroid(const TArray<FVector2D>& Points);
	static TArray<FVector2D> RotateBy(const TArray<FVector2D>& Points, double Radians);
	static double IndicativeAngle(const TArray<FVector2D>& Points);
	static TArray<FVector2D> ScaleToSquare(const TArray<FVector2D>& Points, double Size);
	static TArray<FVector2D> TranslateToOrigin(const TArray<FVector2D>& Points);
	static double PathDistance(const TArray<FVector2D>& PointsA, const TArray<FVector2D>& PointsB);

	double DistanceAtAngle(const TArray<FVector2D>& Points, const TArray<FVector2D>& TemplatePoints, double Radians) const;
	double DistanceAtBestAngle(const TArray<FVector2D>& Points, const TArray<FVector2D>& TemplatePoints, double RadiansA, double RadiansB, double ThetaDeltaRadians) const;
};
