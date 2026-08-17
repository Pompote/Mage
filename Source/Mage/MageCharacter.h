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
struct FInputActionValue;

UCLASS()
class MAGE_API AMageCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMageCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	// Camera vue a la premiere personne, collee a la tete du personnage (pas de bras de camera)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	// Placeholder visuel (capsule du moteur) en attendant un vrai mesh de personnage
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> PlaceholderMesh;

	// Capture le trace souris pendant le dessin de sorts et le fait reconnaitre
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spells", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpellDrawComponent> SpellDrawComponent;

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

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
};
