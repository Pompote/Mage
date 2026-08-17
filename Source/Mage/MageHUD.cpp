// Fill out your copyright notice in the Description page of Project Settings.

#include "MageHUD.h"

#include "Engine/Canvas.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Spells/SpellDrawComponent.h"

void AMageHUD::DrawHUD()
{
	Super::DrawHUD();

	if (Canvas)
	{
		DrawCrosshair(FVector2D(Canvas->SizeX * 0.5f, Canvas->SizeY * 0.5f));
	}

	const APawn* Pawn = PlayerOwner ? PlayerOwner->GetPawn() : nullptr;
	const USpellDrawComponent* DrawComponent = Pawn ? Pawn->FindComponentByClass<USpellDrawComponent>() : nullptr;

	if (!DrawComponent || !DrawComponent->IsDrawingStroke() || !Canvas)
	{
		return;
	}

	const TArray<FVector2D>& Stroke = DrawComponent->GetCurrentStroke();
	if (Stroke.Num() < 2)
	{
		return;
	}

	// Trace centre sur l'ecran : le point de depart du dessin correspond au centre du viewport
	const FVector2D ScreenCenter(Canvas->SizeX * 0.5f, Canvas->SizeY * 0.5f);

	for (int32 Index = 1; Index < Stroke.Num(); ++Index)
	{
		// Pas d'inversion : Stroke.Y arrive deja passe par le modificateur Negate de l'IMC (ajoute
		// pour la camera), le reappliquer ici reintroduirait l'inversion qu'il corrige.
		const FVector2D ScreenStart = ScreenCenter + Stroke[Index - 1];
		const FVector2D ScreenEnd = ScreenCenter + Stroke[Index];
		DrawLine(ScreenStart.X, ScreenStart.Y, ScreenEnd.X, ScreenEnd.Y, FLinearColor::Yellow, 3.f);
	}
}

void AMageHUD::DrawCrosshair(const FVector2D& Center)
{
	const float HalfLength = 8.f;
	const float Thickness = 2.f;

	DrawLine(Center.X - HalfLength, Center.Y, Center.X + HalfLength, Center.Y, FLinearColor::White, Thickness);
	DrawLine(Center.X, Center.Y - HalfLength, Center.X, Center.Y + HalfLength, FLinearColor::White, Thickness);
}
