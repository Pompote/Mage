// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Spells/SpellEffect.h"
#include "ProjectileSpellEffect.generated.h"

class ASpellProjectile;

// Effet generique : fait apparaitre un ASpellProjectile devant la camera du lanceur, dans sa
// direction de visee actuelle (calculee au cast, pas au dessin -- voir README section 4). Le
// visuel/mesh du projectile vient entierement de ProjectileClass : un nouveau sort-projectile ne
// demande qu'un nouveau Blueprint enfant de ce type, jamais de code.
UCLASS(Blueprintable)
class MAGE_API UProjectileSpellEffect : public USpellEffect
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Spell")
	TSubclassOf<ASpellProjectile> ProjectileClass;

	// Distance devant la camera a laquelle le projectile apparait -- evite qu'il naisse a
	// l'interieur de la capsule du lanceur
	UPROPERTY(EditDefaultsOnly, Category = "Spell")
	float SpawnForwardOffset = 100.f;

	virtual void Execute_Implementation(AActor* Caster) override;
};
