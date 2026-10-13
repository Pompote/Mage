// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ElementTypes.generated.h"

// Element porte par un acteur apres avoir ete touche par un sort elementaire, ou par un porteur
// jete (grab/throw). "None" = pas d'element -- comportement no-op partout ou Element est lu (voir
// README section "Mecanique #2 : Elements et interactions").
UENUM(BlueprintType)
enum class EElement : uint8
{
	None	UMETA(DisplayName = "None"),
	Fire	UMETA(DisplayName = "Fire"),
	Water	UMETA(DisplayName = "Water"),
	Earth	UMETA(DisplayName = "Earth"),
	Air	UMETA(DisplayName = "Air"),
	Lightning	UMETA(DisplayName = "Lightning"),
};
