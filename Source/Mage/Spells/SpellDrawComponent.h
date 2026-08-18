// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Spells/GestureRecognizer.h"
#include "SpellDrawComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGestureRecognizedDynamic, const FGestureRecognitionResult&, Result);

// Capture le trace souris pendant un dessin de sort et le fait reconnaitre par USpellRecognitionSubsystem.
// Ne connait ni les sorts, ni le buffer de preparation/declenchement (couche de resolution, plus tard),
// ni comment/ou les formes sont chargees -- seulement "voici un trace, voici ce qui a ete reconnu".
UCLASS(ClassGroup = (Spells), meta = (BlueprintSpawnableComponent))
class MAGE_API USpellDrawComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USpellDrawComponent();

	// A appeler quand le joueur commence a dessiner (touche de dessin pressee)
	void BeginStroke();

	// A appeler quand le joueur relache la touche de dessin ; declenche la reconnaissance
	void EndStroke();

	// A appeler par le personnage avec le delta souris brut tant qu'un trace est en cours -- remplace
	// l'input normalement envoye a la camera pendant le dessin (voir README "Mobilite pendant le dessin")
	void AddStrokeInput(const FVector2D& Delta);

	bool IsDrawingStroke() const { return bIsDrawingStroke; }

	// Vrai pendant le dessin ET pendant une courte fenetre apres la fin du trace -- absorbe le
	// mouvement souris residuel au moment du relachement (independamment de l'ordre d'evaluation
	// entre l'action Draw et l'action Look sur la meme frame), pour eviter un a-coup de camera en
	// fin de dessin. Le personnage doit ignorer completement l'input camera quand ceci est vrai et
	// que IsDrawingStroke() est faux (ne pas non plus l'ajouter au trace, deja termine).
	bool ShouldSuppressCameraInput() const;

	// Points du trace en cours (espace de dessin local, origine au point de depart), pour affichage HUD
	const TArray<FVector2D>& GetCurrentStroke() const { return CurrentStroke; }

	// Diffuse au reste du jeu le resultat d'une reconnaissance -- la couche de resolution des sorts
	// s'y abonnera plus tard pour remplir le buffer de sorts prepares.
	UPROPERTY(BlueprintAssignable, Category = "Spells")
	FOnGestureRecognizedDynamic OnGestureRecognized;

protected:
	// Unites de dessin par unite de delta souris -- reglage de sensation, a ajuster en testant
	UPROPERTY(EditAnywhere, Category = "Spells")
	float StrokeSensitivity = 10.f;

	// Duree (secondes) pendant laquelle l'input camera reste ignore apres la fin d'un trace
	UPROPERTY(EditAnywhere, Category = "Spells")
	float PostStrokeGraceSeconds = 0.15f;

private:
	bool bIsDrawingStroke = false;
	FVector2D PenPosition = FVector2D::ZeroVector;
	TArray<FVector2D> CurrentStroke;

	// < 0 tant qu'aucun trace n'a encore ete termine
	double LastStrokeEndTimeSeconds = -1.0;
};
