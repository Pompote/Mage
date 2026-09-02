// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Elements/ElementTypes.h"
#include "GameFramework/Actor.h"
#include "SpellProjectile.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;

// Projectile generique lance par un effet de sort (voir UProjectileSpellEffect) : vole tout droit
// depuis son point d'apparition, se detruit au premier impact. Le visuel (mesh, echelle) se regle
// entierement depuis un Blueprint enfant -- aucun mesh assigne par defaut ici.
UCLASS()
class MAGE_API ASpellProjectile : public AActor
{
	GENERATED_BODY()

public:
	ASpellProjectile();

	// Element porte par ce projectile, livre a la cible au moment de l'impact (voir OnHit) --
	// assigne par UProjectileSpellEffect au spawn. None = pas de statut a livrer (comportement
	// inchange pour un sort sans element).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spells")
	EElement Element = EElement::None;

	UFUNCTION(BlueprintCallable, Category = "Spells")
	void SetElement(EElement InElement) { Element = InElement; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spells", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spells", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spells", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);
};
