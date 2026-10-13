// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Elements/ElementTypes.h"
#include "GameFramework/Actor.h"
#include "SurfaceElementResidue.generated.h"

class UElementalStatusComponent;
class UStaticMeshComponent;

// Reste generique d'un element qui persiste au sol apres l'impact d'un sort "Toutes surfaces" (voir
// ASpellProjectile::OnHit, USpellDefinition::bPersistsOnAnySurface). Porte son propre
// UElementalStatusComponent -- le meme composant que tout autre porteur de statut -- donc participe
// a la resolution de reaction (y compris avec le vent/une propagation, une fois ce systeme construit)
// exactement comme n'importe quel acteur, sans code de reaction supplementaire. Voir
// Doc/claude-plan.md.
//
// Se detruit automatiquement des que son statut retombe a None (expiration naturelle, ou reaction
// avec un element different qui arrive dessus).
UCLASS(Blueprintable)
class MAGE_API ASurfaceElementResidue : public AActor
{
	GENERATED_BODY()

public:
	ASurfaceElementResidue();

	// Pose Element (livre par SpellId, voir USpellDefinition::SpellId) sur ce residu.
	UFUNCTION(BlueprintCallable, Category = "Element")
	void ImprintElement(EElement Element, FName SpellId);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Element", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Element", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UElementalStatusComponent> ElementalStatus;

private:
	UFUNCTION()
	void HandleElementChanged(EElement NewElement);
};
