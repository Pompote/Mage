// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GrabThrowComponent.generated.h"

// Sort de manipulation, toujours disponible (pas dessine, pas dans le buffer de sorts) -- voir
// README "Mecanique #2 : Elements et interactions". Saisit un acteur IGrabbableInterface a portee,
// le maintient devant la camera du porteur, puis le lance sur relachement. Ne connait rien de
// l'element transporte -- c'est l'acteur saisi lui-meme (ex: AMageEnemyCharacter) qui gere sa
// propre livraison d'element a l'impact.
UCLASS(ClassGroup = (Grab), meta = (BlueprintSpawnableComponent))
class MAGE_API UGrabThrowComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGrabThrowComponent();

	// Distance max du trace de saisie
	UPROPERTY(EditAnywhere, Category = "Grab")
	float GrabRange = 300.f;

	// Rayon du sphere trace de saisie -- plus tolerant qu'un simple line trace
	UPROPERTY(EditAnywhere, Category = "Grab")
	float GrabTraceRadius = 40.f;

	// Distance devant la camera a laquelle l'acteur porte est maintenu
	UPROPERTY(EditAnywhere, Category = "Grab")
	float CarryDistance = 150.f;

	// Vitesse (unites/seconde) donnee a l'acteur relache, dans la direction de la camera
	UPROPERTY(EditAnywhere, Category = "Grab")
	float ThrowSpeed = 1500.f;

	// Cherche un acteur saisissable devant la camera et le saisit s'il y en a un a portee. Ne fait
	// rien si un acteur est deja tenu.
	void TryGrab();

	// Relache l'acteur actuellement tenu avec une impulsion dans la direction de la camera. Ne fait
	// rien si rien n'est tenu.
	void ReleaseOrThrow();

	bool IsHoldingSomething() const { return HeldActor.IsValid(); }

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	TWeakObjectPtr<AActor> HeldActor;
};
