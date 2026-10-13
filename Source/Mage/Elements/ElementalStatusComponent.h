// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Elements/ElementalAffinityProfile.h"
#include "Elements/ElementTypes.h"
#include "ElementalStatusComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnElementChanged, EElement, NewElement);

// Porte l'element actuellement applique a cet acteur (statut a duree limitee). Point d'entree
// unique pour "un element touche cet acteur", que la source soit un sort ou un porteur jete
// (grab/lancer) -- voir README section "Mecanique #2 : Elements et interactions". C'est ce
// composant qui decide, via AffinityProfile, combien de temps l'element tient, et qui declenchera
// la resolution de reaction si un DEUXIEME element different rencontre celui deja present.
// Le UElementReactionSubsystem n'existe pas encore (phase C du plan) : la branche reaction se
// contente de logguer pour l'instant, voir ApplyElement.
UCLASS(ClassGroup = (Elements), meta = (BlueprintSpawnableComponent))
class MAGE_API UElementalStatusComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UElementalStatusComponent();

	// Profil d'affinite de ce type d'acteur -- EditAnywhere (pas EditDefaultsOnly) pour permettre
	// une variante par instance sans cout supplementaire
	UPROPERTY(EditAnywhere, Category = "Element")
	TObjectPtr<UElementalAffinityProfile> AffinityProfile;

	// Duree (secondes) d'un element applique avec un comportement d'affinite Normal (x1.0) --
	// reglage global, multiplie par DurationMultiplier de l'entree d'affinite correspondante
	UPROPERTY(EditAnywhere, Category = "Element")
	float BaseElementDurationSeconds = 6.f;

	// Applique NewElement (pose par le sort/porteur identifie par NewSpellId, voir
	// USpellDefinition::SpellId -- None si l'appelant n'a pas d'identite de sort a donner) a cet
	// acteur :
	//  - aucun element present -> pose le statut, duree = base * multiplicateur d'affinite
	//  - meme element deja present -> rafraichit simplement la duree (et retient NewSpellId comme
	//    source la plus recente)
	//  - element DIFFERENT deja present -> resout la reaction via UElementReactionSubsystem, d'abord
	//    au niveau du sort (CurrentSpellId/NewSpellId) puis, a defaut, au niveau de l'element
	//    (CurrentElement/NewElement) -- voir Doc/claude-plan.md
	UFUNCTION(BlueprintCallable, Category = "Element")
	void ApplyElement(EElement NewElement, FName NewSpellId = NAME_None, AActor* SourceActor = nullptr, FVector ContactLocation = FVector::ZeroVector);

	UFUNCTION(BlueprintPure, Category = "Element")
	EElement GetCurrentElement() const { return CurrentElement; }

	// Identite du sort actuellement responsable du statut porte (voir USpellDefinition::SpellId) --
	// None si aucun statut, ou si le statut actuel vient d'un resultat de reaction compose plutot
	// que directement d'un sort (voir ApplyElement).
	UFUNCTION(BlueprintPure, Category = "Element")
	FName GetCurrentSpellId() const { return CurrentSpellId; }

	UFUNCTION(BlueprintPure, Category = "Element")
	bool HasElement() const { return CurrentElement != EElement::None; }

	// Diffuse a chaque changement reel d'element (application depuis None, expiration, ou remise a
	// None apres reaction) -- PAS a chaque rafraichissement de duree sur le meme element, puisque
	// l'etat visible ne change pas dans ce cas. Pour VFX/HUD (ex: teinter le mesh) sans avoir a
	// interroger l'etat chaque frame.
	UPROPERTY(BlueprintAssignable, Category = "Element")
	FOnElementChanged OnElementChanged;

	// Efface le statut immediatement (sans attendre l'expiration) -- utilise par exemple par un
	// porteur jete (grab/lancer) une fois son element livre a l'impact : l'element "part" du
	// porteur, qu'une reaction se soit declenchee ou non sur la cible.
	UFUNCTION(BlueprintCallable, Category = "Element")
	void ClearElement();

private:
	EElement CurrentElement = EElement::None;
	FName CurrentSpellId;
	FTimerHandle ExpiryTimerHandle;

	float GetDurationSeconds(EElement Element) const;
	void SetElementInternal(EElement NewElement, FName NewSpellId, float DurationSeconds);

	UFUNCTION()
	void HandleExpiry();
};
