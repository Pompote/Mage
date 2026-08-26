// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MageCharacter.generated.h"

class UCameraComponent;
class UStaticMeshComponent;
class UInputMappingContext;
class UInputAction;
class USpellDrawComponent;
class USpellBufferComponent;
struct FInputActionValue;

UCLASS()
class MAGE_API AMageCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMageCharacter();

	// 0 = dash disponible, 1 = tout juste consomme -- decroit lineairement vers 0 au fil du
	// cooldown. Pour l'affichage HUD (case qui se remplit de bas en haut).
	float GetDashCooldownRemainingFraction() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	// Camera vue a la premiere personne, collee a la tete du personnage (pas de bras de camera)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	// Fraction (0-1) de la sensibilite camera normale encore appliquee pendant le dessin -- pour
	// garder une legere sensation de "le mage regarde autour de lui" plutot qu'un blocage total.
	// La fenetre de grace juste apres la fin du trace, elle, reste en coupure totale (voir
	// USpellDrawComponent::ShouldSuppressCameraInput).
	UPROPERTY(EditAnywhere, Category = "Camera")
	float CameraAttenuationWhileDrawing = 0.1f;

	// Tremblement qui grandit avec la duree du trace en cours (plafonne) -- amplitude gagnee par
	// seconde de dessin
	UPROPERTY(EditAnywhere, Category = "Camera")
	float DrawShakeGrowthPerSecond = 0.05f;

	// Plafond de l'amplitude du tremblement croissant, quelle que soit la duree du trace
	UPROPERTY(EditAnywhere, Category = "Camera")
	float DrawShakeMaxAmplitude = 0.3f;

	// Vitesse d'oscillation du tremblement croissant
	UPROPERTY(EditAnywhere, Category = "Camera")
	float DrawShakeFrequency = 8.f;

	// Micro-mouvements subtils en directions aleatoires, intensite constante (independante de la
	// duree du trace), pendant tout le dessin
	UPROPERTY(EditAnywhere, Category = "Camera")
	float DrawMicroJitterAmplitude = 0.02f;

	// Nombre de secousses aleatoires par seconde (pas a chaque frame, sinon ca ressemble a un
	// bourdonnement continu plutot qu'a un tremblement) -- chaque secousse est une seule impulsion,
	// tenue jusqu'a la prochaine
	UPROPERTY(EditAnywhere, Category = "Camera")
	float DrawMicroJitterFrequency = 15.f;

	// Vitesse (unites/seconde) a laquelle le dash deplace le personnage -- controle le cote
	// "instantane" du mouvement ; c'est DashRange, pas cette valeur, qui determine si un dash
	// suffit a esquiver une attaque donnee (duree du dash = DashRange / DashSpeed)
	UPROPERTY(EditAnywhere, Category = "Dash")
	float DashSpeed = 3000.f;

	// Distance totale (unites) parcourue par un dash, independamment de sa vitesse
	UPROPERTY(EditAnywhere, Category = "Dash")
	float DashRange = 600.f;

	// Temps minimum (secondes) entre deux dash
	UPROPERTY(EditAnywhere, Category = "Dash")
	float DashCooldownSeconds = 1.5f;

	// Placeholder visuel (capsule du moteur) en attendant un vrai mesh de personnage
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> PlaceholderMesh;

	// Capture le trace souris pendant le dessin de sorts et le fait reconnaitre
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spells", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpellDrawComponent> SpellDrawComponent;

	// Banque de sorts prepares (dessines mais pas encore lances) -- voir README section 4
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spells", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpellBufferComponent> SpellBufferComponent;

	// Assets d'input Enhanced Input, a assigner dans l'editeur (Blueprint enfant conseille)
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	// Digital (bool) : presse pour commencer a dessiner, relache pour reconnaitre le trace
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> DrawAction;

	// Touches 1/2/3 : declenchent le cast reel d'un emplacement du buffer de sorts. Une seule
	// Input Action (Axis1D/float) partagee par les 3 touches -- chaque touche porte un modificateur
	// Scalar different (1.0 / 2.0 / 3.0) dans l'IMC, ce qui permet de savoir laquelle a ete pressee
	// a partir de la seule valeur recue. Augmenter au-dela de 3 emplacements ne demande alors qu'une
	// touche supplementaire dans l'IMC (Scalar 4.0), sans recompiler.
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> CastSlotAction;

	// Digital (bool) : esquive dans la direction du deplacement en cours (ou vers l'avant si aucun
	// input de deplacement), sous cooldown
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> DashAction;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void CastSlot(const FInputActionValue& Value);
	void Dash();

	// < 0 tant qu'aucune secousse n'a encore ete jouee
	double LastJitterUpdateTimeSeconds = -1.0;

	// Direction (normalisee) du dash en cours ; sans effet si DashDistanceRemaining <= 0
	FVector DashDirection = FVector::ZeroVector;

	// Distance qu'il reste a parcourir sur le dash en cours -- <= 0 signifie "pas de dash en cours"
	float DashDistanceRemaining = 0.f;

	// Tres negatif au depart : un dash est disponible des le debut de partie
	double LastDashTimeSeconds = -1000.0;
};
