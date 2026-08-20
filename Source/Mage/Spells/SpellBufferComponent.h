// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SpellBufferComponent.generated.h"

class USpellDefinition;
struct FGestureRecognitionResult;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSpellBufferChanged);

// Recoit les formes reconnues (USpellDrawComponent::OnGestureRecognized) et gere la banque de
// sorts prepares -- voir README section 4 "Preparation et declenchement (buffer de sorts)" :
// dessiner prepare un sort dans le buffer (FIFO si plein), une touche dediee declenche le cast
// reel d'un emplacement. Ne connait ni le dessin/la reconnaissance (juste l'evenement final), ni
// comment le HUD affiche le contenu du buffer.
UCLASS(ClassGroup = (Spells), meta = (BlueprintSpawnableComponent))
class MAGE_API USpellBufferComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USpellBufferComponent();

	// Nombre max de sorts prepares simultanement -- valeur d'instance, ajustable sans recompiler
	UPROPERTY(EditAnywhere, Category = "Spells")
	int32 MaxBufferSize = 3;

	// Tous les sorts que ce personnage est susceptible de reconnaitre -- lookup ShapeId -> Sort.
	// Simple liste geree a la main (comme pour les formes), pas de decouverte automatique.
	UPROPERTY(EditAnywhere, Category = "Spells")
	TArray<TObjectPtr<USpellDefinition>> KnownSpells;

	// Sorts actuellement prepares, dans l'ordre d'arrivee (index 0 = le plus ancien)
	const TArray<TObjectPtr<USpellDefinition>>& GetBufferedSpells() const { return BufferedSpells; }

	// Declenche reellement le sort a cet emplacement (retire du buffer) ; ne fait rien si vide
	UFUNCTION(BlueprintCallable, Category = "Spells")
	void CastSlot(int32 SlotIndex);

	// Diffuse chaque fois que le contenu du buffer change (ajout ou cast) -- le HUD peut aussi
	// simplement relire GetBufferedSpells() chaque frame ; ce delegate est un bonus si on veut
	// reagir sans polling plus tard.
	UPROPERTY(BlueprintAssignable, Category = "Spells")
	FOnSpellBufferChanged OnBufferChanged;

protected:
	virtual void BeginPlay() override;

private:
	TArray<TObjectPtr<USpellDefinition>> BufferedSpells;

	UFUNCTION()
	void HandleGestureRecognized(const FGestureRecognitionResult& Result);
};
