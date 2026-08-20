// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SpellEffect.generated.h"

// Ce que fait reellement un sort au moment du cast (degats, soin, buff...). Ne fait rien par
// defaut -- stub pense pour que USpellBufferComponent::CastSlot puisse deja appeler un effet des
// maintenant, sans que sa logique n'ait besoin de changer quand on implementera de vrais effets.
UCLASS(Abstract, Blueprintable)
class MAGE_API USpellEffect : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, Category = "Spell")
	void Execute(AActor* Caster);
	virtual void Execute_Implementation(AActor* Caster);
};
