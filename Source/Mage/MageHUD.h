// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MageHUD.generated.h"

// Affiche le trace en cours pendant le dessin d'un sort, en lisant USpellDrawComponent sur le pawn
// possede. Purement visuel -- ne participe pas a la reconnaissance.
UCLASS()
class MAGE_API AMageHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

protected:
	// Croix de visee au centre de l'ecran -- independante du choix ecran/monde ci-dessous, a
	// desactiver pour voir le trace 3D sans repere ecran superpose (utile pour juger la sensation
	// a distance)
	UPROPERTY(EditAnywhere, Category = "Spells")
	bool bShowCrosshair = true;

	// Delai (secondes) entre le trace reel et ce qui s'affiche -- purement visuel, la reconnaissance
	// utilise toujours les points reels sans delai. Une fois le dessin termine, l'affichage continue
	// de "rattraper" le trace reel pendant ce meme delai avant de disparaitre.
	UPROPERTY(EditAnywhere, Category = "Spells")
	float TraceDelaySeconds = 0.08f;

	// Amplitude (pixels) du tremblement organique ajoute a l'affichage du trace
	UPROPERTY(EditAnywhere, Category = "Spells")
	float TraceNoiseAmplitude = 4.f;

	// Vitesse d'oscillation du tremblement dans le temps
	UPROPERTY(EditAnywhere, Category = "Spells")
	float TraceNoiseSpeed = 6.f;

	// Frequence spatiale du tremblement le long du trace (dephasage entre points consecutifs)
	UPROPERTY(EditAnywhere, Category = "Spells")
	float TraceNoiseFrequency = 0.4f;

	// Debug/test : affiche un petit marqueur a la position reelle du "stylo" pendant le dessin
	// (position brute, sans le delai ni le bruit du trait) -- a activer/desactiver depuis un
	// Blueprint enfant (BP_MageHUD) selon si on decide de le garder ou non.
	UPROPERTY(EditAnywhere, Category = "Spells")
	bool bShowDebugCursor = true;

	// Si vrai, le trace est projete en 3D devant la camera (via DrawDebugLine, outil de debug --
	// desactive en Shipping) plutot que dessine a plat en espace ecran. A remplacer par un vrai
	// mesh/Niagara avant de sortir le jeu si on garde cette approche.
	UPROPERTY(EditAnywhere, Category = "Spells")
	bool bDrawTraceInWorldSpace = true;

	// Distance (unites Unreal) devant la camera a laquelle le plan de dessin 3D est place
	UPROPERTY(EditAnywhere, Category = "Spells", meta = (EditCondition = "bDrawTraceInWorldSpace"))
	float TraceWorldDistance = 150.f;

	// Echelle appliquee aux unites de l'espace de dessin (StrokeSensitivity) pour obtenir une
	// taille de geste raisonnable en unites Unreal
	UPROPERTY(EditAnywhere, Category = "Spells", meta = (EditCondition = "bDrawTraceInWorldSpace"))
	float TraceWorldScale = 0.4f;

	// Taille (pixels) de chaque icone du buffer de sorts, bas-gauche de l'ecran
	UPROPERTY(EditAnywhere, Category = "Spells")
	float BufferIconSize = 48.f;

	// Espace (pixels) entre deux icones du buffer
	UPROPERTY(EditAnywhere, Category = "Spells")
	float BufferIconSpacing = 8.f;

	// Marge (pixels) depuis le bord bas et le bord gauche de l'ecran
	UPROPERTY(EditAnywhere, Category = "Spells")
	float BufferMarginFromEdge = 24.f;

private:
	// Petite croix au centre de l'ecran, repere de vue en permanence (et point de depart du dessin)
	void DrawCrosshair(const FVector2D& Center);

	// Offset organique purement visuel pour le point d'index PointIndex, au temps CurrentTime
	FVector2D GetNoiseOffset(int32 PointIndex, double CurrentTime) const;

	// Convertit un point d'espace de dessin (2D) en position monde, sur le plan devant la camera
	FVector ProjectToWorld(const FVector2D& DrawSpacePoint) const;

	// Rangee d'icones du buffer de sorts en bas a gauche -- lu en direct depuis USpellBufferComponent
	// chaque frame, donc toujours a jour des qu'un sort est prepare ou lance
	void DrawSpellBuffer(const class APawn* Pawn);
};
