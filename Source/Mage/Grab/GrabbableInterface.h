// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GrabbableInterface.generated.h"

UINTERFACE(BlueprintType)
class MAGE_API UGrabbableInterface : public UInterface
{
	GENERATED_BODY()
};

// Capacite "peut etre saisi/porte/lance" (voir README "Mecanique #2 : Elements et interactions",
// sort de manipulation). Interface plutot que base commune avec ASpellProjectile -- n'importe quel
// type d'acteur peut l'implementer sans heriter d'une classe partagee.
class MAGE_API IGrabbableInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, Category = "Grab")
	bool CanBeGrabbed() const;
	virtual bool CanBeGrabbed_Implementation() const;

	UFUNCTION(BlueprintNativeEvent, Category = "Grab")
	void OnGrabbed(AActor* Grabber);
	virtual void OnGrabbed_Implementation(AActor* Grabber);

	UFUNCTION(BlueprintNativeEvent, Category = "Grab")
	void OnThrown(AActor* Thrower, const FVector& ThrowVelocity);
	virtual void OnThrown_Implementation(AActor* Thrower, const FVector& ThrowVelocity);
};
