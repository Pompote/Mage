// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ElementReactionEffectActor.generated.h"

// Placeholder generique pour le resultat visuel d'une reaction elementaire (Brume, Boue, etc.) --
// une sphere de debug qui s'efface au bout de LifetimeSeconds. A remplacer par un vrai mesh/Niagara
// par ligne de UElementReactionTable une fois qu'on aura de vrais assets (voir README "Mecanique #2").
UCLASS(Blueprintable)
class MAGE_API AElementReactionEffectActor : public AActor
{
	GENERATED_BODY()

public:
	AElementReactionEffectActor();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Element")
	float LifetimeSeconds = 2.f;

	UPROPERTY(EditAnywhere, Category = "Element")
	FColor DebugColor = FColor::White;
};
