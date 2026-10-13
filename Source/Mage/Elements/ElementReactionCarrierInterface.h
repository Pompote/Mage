// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ElementReactionCarrierInterface.generated.h"

UINTERFACE(BlueprintType)
class MAGE_API UElementReactionCarrier : public UInterface
{
	GENERATED_BODY()
};

// Capacite "je suis la source d'un element qui vient de declencher une reaction sur sa cible" --
// notifie apres coup (voir UElementReactionSubsystem::TryReact), pour qu'un porteur puisse reagir a
// son tour (ex: un projectile spawn un effet de remplacement a trajectoire modifiee au lieu de
// simplement disparaitre, voir README table des combinaisons par sort : "Boule de feu + Eclair ->
// ... trajectoire eclair, disparition boule de feu" -- c'est l'Eclair, le porteur entrant, qui
// continue). Interface plutot que base commune avec ASpellProjectile -- meme precedent que
// IGrabbableInterface (voir Grab/GrabbableInterface.h) : n'importe quel type d'acteur peut s'en
// servir sans heriter d'une classe partagee.
//
// Implementation par defaut vide : ne change rien au comportement existant tant que rien ne la
// surcharge. ASpellProjectile::OnHit se detruit toujours inconditionnellement, independamment de
// cette notification -- voir Doc/claude-plan.md pour le raisonnement (le hook sert a faire apparaitre
// quelque chose en reaction, pas a empecher la destruction du porteur d'origine).
class MAGE_API IElementReactionCarrier
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, Category = "Element")
	void OnDeliveredElementReacted(FName ReactionId, AActor* TargetActor);
	virtual void OnDeliveredElementReacted_Implementation(FName ReactionId, AActor* TargetActor);
};
