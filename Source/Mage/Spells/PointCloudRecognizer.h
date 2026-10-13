// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Spells/GestureRecognizer.h"
#include "PointCloudRecognizer.generated.h"

// Implementation du $P Point-Cloud Recognizer (Vatavu, Anthony, Wobbrock - 2012) : reechantillonnage,
// rotation a l'angle indicatif, mise a l'echelle UNIFORME (contrairement au $1, qui deforme X/Y
// independamment), puis correspondance gloutonne ponderee entre nuages de points (sans notion
// d'ordre/alignement le long du trait). Remplace UDollarOneRecognizer comme implementation par
// defaut -- voir Doc/claude-plan.md : mesure sur tracés a la souris reels (pas juste les gestes
// generes) montre que $1 confond des silhouettes fermees de proportions proches (cercle/carre/
// triangle a moins de 0.05 d'ecart) et peut faire gagner une forme simple (Line) sur un tracé
// imprecis d'une forme plus complexe (Zigzag) ; $P corrige les deux sur les memes tracés de test.
UCLASS(BlueprintType)
class MAGE_API UPointCloudRecognizer : public UObject, public IGestureRecognizer
{
	GENERATED_BODY()

public:
	virtual TArray<FVector2D> Normalize(const TArray<FVector2D>& RawPoints) const override;
	virtual FGestureRecognitionResult Recognize(const TArray<FVector2D>& RawStroke, const TArray<FGestureTemplate>& Templates) const override;

protected:
	// Nombre de points apres reechantillonnage -- doit rester coherent entre le bake (voir
	// GestureShapeTemplate::BakeFromImage) et la reconnaissance en jeu.
	UPROPERTY(EditAnywhere, Category = "Point Cloud")
	int32 NumResamplePoints = 64;

	// Taille du carre de reference pour la mise a l'echelle (uniforme, aspect ratio preserve)
	UPROPERTY(EditAnywhere, Category = "Point Cloud")
	float SquareSize = 250.f;

	// Seuil plancher (0-1) : le score $P discrimine surtout par CLASSEMENT (quel template gagne),
	// pas par un ecart net entre bon et mauvais candidat (les scores restent tous dans une plage
	// resserree, ex. 0.88-0.98 mesure sur les tracés de test) -- ce seuil sert seulement a rejeter
	// un gribouillis qui ne ressemble a aucun template, pas a departager des formes proches.
	UPROPERTY(EditAnywhere, Category = "Point Cloud")
	float RecognitionThreshold = 0.75f;

private:
	static double PathLength(const TArray<FVector2D>& Points);
	static TArray<FVector2D> Resample(const TArray<FVector2D>& Points, int32 N);
	static FVector2D Centroid(const TArray<FVector2D>& Points);
	static TArray<FVector2D> RotateBy(const TArray<FVector2D>& Points, double Radians);
	static double IndicativeAngle(const TArray<FVector2D>& Points);
	static TArray<FVector2D> ScaleToSquareUniform(const TArray<FVector2D>& Points, double Size);
	static TArray<FVector2D> TranslateToOrigin(const TArray<FVector2D>& Points);

	// Distance d'un nuage vers l'autre en partant de l'index Start : associe glucidement chaque
	// point de PointsA au plus proche point encore disponible de PointsB, pondere par la distance
	// (en indice circulaire) au point de depart -- favorise les correspondances proches du point de
	// depart choisi, sans imposer un ordre de parcours strict comme le ferait $1.
	static double CloudDistance(const TArray<FVector2D>& PointsA, const TArray<FVector2D>& PointsB, int32 N, int32 Start);

	// Cherche le meilleur point de depart (echantillonne, pas exhaustif -- N^(1-eps) essais comme
	// dans l'article original) et le sens (A->B ou B->A), retourne la distance moyenne par point.
	static double GreedyCloudMatch(const TArray<FVector2D>& PointsA, const TArray<FVector2D>& PointsB, int32 N);
};
