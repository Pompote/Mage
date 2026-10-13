// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Elements/ElementTypes.h"
#include "Engine/DataAsset.h"
#include "ElementalAffinityProfile.generated.h"

// Comportement d'un type d'ennemi face a un element donne -- extensible sans toucher au code de
// UElementalStatusComponent::ApplyElement, seulement cette liste.
UENUM(BlueprintType)
enum class EElementAffinityBehavior : uint8
{
	Normal		UMETA(DisplayName = "Normal"),		// duree standard (ex: gobelin + feu)
	Absorbent	UMETA(DisplayName = "Absorbent"),	// duree prolongee -- bon "reservoir" a jeter (ex: golem d'argile + eau)
};

USTRUCT(BlueprintType)
struct FElementAffinityEntry
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Element")
	EElement Element = EElement::None;

	UPROPERTY(EditDefaultsOnly, Category = "Element")
	EElementAffinityBehavior Behavior = EElementAffinityBehavior::Normal;

	// Multiplie la duree de base (UElementalStatusComponent::BaseElementDurationSeconds) -- 1.0 =
	// standard, >1.0 pour un comportement Absorbent (ex: 3.0)
	UPROPERTY(EditDefaultsOnly, Category = "Element")
	float DurationMultiplier = 1.f;

	// Multiplie les degats infliges par cet element -- 1.0 = standard, >1.0 = faiblesse (ex: 2.0),
	// <1.0 = resistance (ex: 0.5). Voir README section "Ennemis" (faiblesses/resistances par type).
	// Rien ne consomme ce champ pour l'instant : aucun systeme de degats/PV n'existe encore dans le
	// projet (voir Doc/claude-plan.md) -- le champ existe pour qu'un futur systeme de combat ait
	// quelque chose a lire, et pour pouvoir deja renseigner de vrais chiffres sur les profils.
	UPROPERTY(EditDefaultsOnly, Category = "Element")
	float DamageMultiplier = 1.f;
};

// Table d'affinite d'un type d'ennemi : une entree par element pertinent. Un element absent de la
// liste se comporte comme Normal / x1.0 -- pas un cas particulier a coder ailleurs. Nouveau type
// d'ennemi = nouvel asset + nouvelles lignes, jamais de branchement C++ par type.
UCLASS(BlueprintType)
class MAGE_API UElementalAffinityProfile : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Element")
	TArray<FElementAffinityEntry> Affinities;

	// Retourne l'entree correspondant a Element, ou {Element, Normal, 1.0} si absent de la liste
	UFUNCTION(BlueprintPure, Category = "Element")
	FElementAffinityEntry GetAffinity(EElement Element) const;
};
