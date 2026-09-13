// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Elements/ElementTypes.h"
#include "GameFramework/Character.h"
#include "Grab/GrabbableInterface.h"
#include "MageEnemyCharacter.generated.h"

class UElementalStatusComponent;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class UStaticMeshComponent;

// Etat de deplacement de l'ennemi -- determine ce que fait Tick() chaque frame
UENUM()
enum class EEnemyMotionState : uint8
{
	Seeking,	// marche vers le joueur le plus proche (comportement par defaut)
	Held,		// porte par un joueur (grab) -- Tick ne fait rien, c'est UGrabThrowComponent qui repositionne cet acteur
	Thrown,		// en vol apres un lancer -- Tick avance et detecte l'impact
};

// Ennemi de test minimal : marche vers le joueur, pas de PV/combat, juste un profil elementaire --
// voir README "Mecanique #2 : Elements et interactions", "Terrain de jeux dans la scene de base".
// Sert a jouer la boucle grab -> porter -> lancer, pas a prefigurer le vrai roster d'ennemis.
UCLASS()
class MAGE_API AMageEnemyCharacter : public ACharacter, public IGrabbableInterface
{
	GENERATED_BODY()

public:
	AMageEnemyCharacter();

	virtual bool CanBeGrabbed_Implementation() const override;
	virtual void OnGrabbed_Implementation(AActor* Grabber) override;
	virtual void OnThrown_Implementation(AActor* Thrower, const FVector& ThrowVelocity) override;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Placeholder visuel (capsule du moteur), meme logique que AMageCharacter::PlaceholderMesh
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> PlaceholderMesh;

	// Porte l'element actuellement applique a cet ennemi -- assigner un AffinityProfile dans
	// l'editeur (ex: DA_Affinity_Goblin) pour regler son comportement face a chaque element
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Element", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UElementalStatusComponent> ElementalStatus;

	// Materiau de base a partir duquel un materiau dynamique est cree (voir BeginPlay) pour teinter
	// le mesh selon l'element porte -- assigner M_ElementTint (ou equivalent exposant un parametre
	// vecteur nomme "Color") dans l'editeur. A la difference du tableau "Override Materials" du
	// composant mesh (peu fiable dans l'editeur lors des tests), cette simple reference se cherche
	// et s'assigne comme AffinityProfile/ProjectileClass ailleurs dans le projet.
	UPROPERTY(EditDefaultsOnly, Category = "Element")
	TObjectPtr<UMaterialInterface> BaseTintMaterial;

	// Materiau dynamique cree a partir de BaseTintMaterial au demarrage
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Element", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMaterialInstanceDynamic> TintMaterial;

	// Vitesse de marche vers le joueur le plus proche
	UPROPERTY(EditAnywhere, Category = "AI")
	float WalkSpeed = 200.f;

	// Distance en dessous de laquelle l'ennemi arrete d'avancer (evite qu'il colle/pousse le joueur)
	UPROPERTY(EditAnywhere, Category = "AI")
	float StopDistance = 150.f;

	// Vitesse (unites/seconde) a laquelle cet ennemi vole une fois lance
	UPROPERTY(EditAnywhere, Category = "Grab")
	float ThrownFlightSpeed = 1500.f;

private:
	EEnemyMotionState MotionState = EEnemyMotionState::Seeking;

	// Vitesse courante en vol -- pas juste la direction initiale du lancer : TickThrown lui ajoute
	// la gravite chaque frame pour produire un arc, puisque ce vol n'est pas gere par
	// UCharacterMovementComponent (voir commentaire de TickThrown).
	FVector ThrownVelocity = FVector::ZeroVector;

	void TickSeeking(float DeltaTime);
	void TickThrown(float DeltaTime);

	UFUNCTION()
	void HandleElementChanged(EElement NewElement);
};
