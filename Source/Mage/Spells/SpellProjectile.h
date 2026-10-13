// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Elements/ElementReactionCarrierInterface.h"
#include "Elements/ElementTypes.h"
#include "GameFramework/Actor.h"
#include "SpellProjectile.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;
class ASurfaceElementResidue;

// Projectile generique lance par un effet de sort (voir UProjectileSpellEffect) : vole tout droit
// depuis son point d'apparition. Au premier impact : livre son element (si non-None, et si
// bImprintsEnemy) a une cible porteuse de statut ; sinon, si bPersistsOnAnySurface, laisse un
// ASurfaceElementResidue a la place. Se detruit inconditionnellement ensuite dans tous les cas (voir
// Doc/claude-plan.md -- IElementReactionCarrier ne change pas cette destruction, il sert seulement a
// notifier qu'une reaction a eu lieu). Le visuel (mesh, echelle) se regle entierement depuis un
// Blueprint enfant -- aucun mesh assigne par defaut ici.
UCLASS()
class MAGE_API ASpellProjectile : public AActor, public IElementReactionCarrier
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

	// Identite du USpellDefinition qui a lance ce projectile -- assigne par UProjectileSpellEffect au
	// spawn (voir USpellDefinition::SpellId). Cle de recherche des reactions specifiques a ce sort ;
	// None = pas d'identite connue, seule la table par element s'applique en cas de reaction.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spells")
	FName SpellId;

	UFUNCTION(BlueprintCallable, Category = "Spells")
	void SetSpellId(FName InSpellId) { SpellId = InSpellId; }

	// Si faux, ce projectile n'imprime pas son element sur une cible touchee -- assigne par
	// UProjectileSpellEffect au spawn (voir USpellDefinition::bImprintsEnemy). Vrai par defaut :
	// comportement actuel de tous les sorts elementaires.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spells")
	bool bImprintsEnemy = true;

	UFUNCTION(BlueprintCallable, Category = "Spells")
	void SetImprintsEnemy(bool bInImprintsEnemy) { bImprintsEnemy = bInImprintsEnemy; }

	// Si vrai, un impact sur du decor (pas une cible porteuse de statut) laisse un
	// ASurfaceElementResidue plutot que de disparaitre sans rien laisser -- assigne par
	// UProjectileSpellEffect au spawn (voir USpellDefinition::bPersistsOnAnySurface).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spells")
	bool bPersistsOnAnySurface = false;

	UFUNCTION(BlueprintCallable, Category = "Spells")
	void SetPersistsOnAnySurface(bool bInPersists) { bPersistsOnAnySurface = bInPersists; }

	// Classe du residu spawn quand bPersistsOnAnySurface est vrai -- ASurfaceElementResidue de base
	// par defaut (voir constructeur), overridable par sort via un Blueprint enfant sans code.
	UPROPERTY(EditDefaultsOnly, Category = "Spells")
	TSubclassOf<ASurfaceElementResidue> ResidueClass;

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
