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

private:
	// Petite croix au centre de l'ecran, repere de vue en permanence (et point de depart du dessin)
	void DrawCrosshair(const FVector2D& Center);
};
