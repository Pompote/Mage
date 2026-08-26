// Fill out your copyright notice in the Description page of Project Settings.

#include "MageHUD.h"

#include "DrawDebugHelpers.h"
#include "Engine/Canvas.h"
#include "Engine/Texture2D.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "MageCharacter.h"
#include "Spells/SpellBufferComponent.h"
#include "Spells/SpellDefinition.h"
#include "Spells/SpellDrawComponent.h"

void AMageHUD::DrawHUD()
{
	Super::DrawHUD();

	if (Canvas && bShowCrosshair)
	{
		DrawCrosshair(FVector2D(Canvas->SizeX * 0.5f, Canvas->SizeY * 0.5f));
	}

	const APawn* Pawn = PlayerOwner ? PlayerOwner->GetPawn() : nullptr;

	DrawSpellBuffer(Pawn);
	DrawDashCooldown(Pawn);

	const USpellDrawComponent* DrawComponent = Pawn ? Pawn->FindComponentByClass<USpellDrawComponent>() : nullptr;

	if (!DrawComponent || !Canvas)
	{
		return;
	}

	const TArray<FVector2D>& Stroke = DrawComponent->GetCurrentStroke();
	const TArray<double>& Timestamps = DrawComponent->GetStrokeTimestamps();
	if (Stroke.Num() < 2 || Timestamps.Num() != Stroke.Num())
	{
		return;
	}

	const UWorld* World = GetWorld();
	const double Now = World ? World->GetTimeSeconds() : 0.0;
	const double CutoffTime = Now - static_cast<double>(TraceDelaySeconds);
	const bool bStillCatchingUp = CutoffTime < Timestamps.Last();

	// En dehors du dessin actif, on ne montre plus rien une fois que l'affichage retarde a
	// rattrape le trace reel -- sinon la derniere forme dessinee resterait affichee indefiniment.
	if (!DrawComponent->IsDrawingStroke() && !bStillCatchingUp)
	{
		return;
	}

	// Ne garde que les points dont l'horodatage est deja "passe" le delai d'affichage
	int32 VisibleCount = 1;
	for (int32 Index = 1; Index < Timestamps.Num(); ++Index)
	{
		if (Timestamps[Index] > CutoffTime)
		{
			break;
		}
		VisibleCount = Index + 1;
	}

	if (VisibleCount < 2)
	{
		return;
	}

	if (bDrawTraceInWorldSpace)
	{
		const UWorld* DebugWorld = GetWorld();
		if (!DebugWorld)
		{
			return;
		}

		if (bShowDebugCursor && DrawComponent->IsDrawingStroke())
		{
			DrawDebugSphere(DebugWorld, ProjectToWorld(Stroke.Last()), 3.f, 8, FColor::Red, false, 0.f, 0, 1.f);
		}

		for (int32 Index = 1; Index < VisibleCount; ++Index)
		{
			const FVector2D NoisedStart = Stroke[Index - 1] + GetNoiseOffset(Index - 1, Now);
			const FVector2D NoisedEnd = Stroke[Index] + GetNoiseOffset(Index, Now);
			DrawDebugLine(DebugWorld, ProjectToWorld(NoisedStart), ProjectToWorld(NoisedEnd), FColor::Yellow, false, 0.f, 0, 3.f);
		}
		return;
	}

	// Trace centre sur l'ecran : le point de depart du dessin correspond au centre du viewport
	const FVector2D ScreenCenter(Canvas->SizeX * 0.5f, Canvas->SizeY * 0.5f);

	// Debug/test : position reelle du stylo (brute, sans delai ni bruit), uniquement pendant le dessin actif
	if (bShowDebugCursor && DrawComponent->IsDrawingStroke())
	{
		const FVector2D CursorScreenPos = ScreenCenter + Stroke.Last();
		const float CursorSize = 6.f;
		DrawRect(FLinearColor::Red, CursorScreenPos.X - CursorSize * 0.5f, CursorScreenPos.Y - CursorSize * 0.5f, CursorSize, CursorSize);
	}

	for (int32 Index = 1; Index < VisibleCount; ++Index)
	{
		// Pas d'inversion : Stroke.Y arrive deja passe par le modificateur Negate de l'IMC (ajoute
		// pour la camera), le reappliquer ici reintroduirait l'inversion qu'il corrige.
		const FVector2D ScreenStart = ScreenCenter + Stroke[Index - 1] + GetNoiseOffset(Index - 1, Now);
		const FVector2D ScreenEnd = ScreenCenter + Stroke[Index] + GetNoiseOffset(Index, Now);
		DrawLine(ScreenStart.X, ScreenStart.Y, ScreenEnd.X, ScreenEnd.Y, FLinearColor::Yellow, 3.f);
	}
}

FVector AMageHUD::ProjectToWorld(const FVector2D& DrawSpacePoint) const
{
	if (!PlayerOwner)
	{
		return FVector::ZeroVector;
	}

	FVector CameraLocation;
	FRotator CameraRotation;
	PlayerOwner->GetPlayerViewPoint(CameraLocation, CameraRotation);

	const FRotationMatrix RotMatrix(CameraRotation);
	const FVector Forward = RotMatrix.GetUnitAxis(EAxis::X);
	const FVector Right = RotMatrix.GetUnitAxis(EAxis::Y);
	const FVector Up = RotMatrix.GetUnitAxis(EAxis::Z);

	const FVector PlaneCenter = CameraLocation + Forward * TraceWorldDistance;

	// DrawSpacePoint.Y positif = vers le bas (meme convention que l'affichage ecran, deja verifiee
	// correcte) -- d'ou le signe negatif ici pour "vers le bas" en 3D (Up est positif vers le haut).
	// Si ca parait inverse en jeu, c'est ce signe qu'il faut retourner en premier.
	return PlaneCenter
		+ Right * (DrawSpacePoint.X * TraceWorldScale)
		- Up * (DrawSpacePoint.Y * TraceWorldScale);
}

FVector2D AMageHUD::GetNoiseOffset(int32 PointIndex, double CurrentTime) const
{
	// Ondulation qui se propage le long du trace (dephasage par index) et evolue dans le temps --
	// purement decoratif, jamais utilise pour la reconnaissance.
	const double Phase = PointIndex * static_cast<double>(TraceNoiseFrequency) + CurrentTime * static_cast<double>(TraceNoiseSpeed);
	return FVector2D(FMath::Sin(Phase), FMath::Cos(Phase * 1.3)) * TraceNoiseAmplitude;
}

void AMageHUD::DrawSpellBuffer(const APawn* Pawn)
{
	const USpellBufferComponent* BufferComponent = Pawn ? Pawn->FindComponentByClass<USpellBufferComponent>() : nullptr;
	if (!BufferComponent || !Canvas)
	{
		return;
	}

	const TArray<TObjectPtr<USpellDefinition>>& Buffered = BufferComponent->GetBufferedSpells();
	const float StartY = Canvas->SizeY - BufferMarginFromEdge - BufferIconSize;

	for (int32 Index = 0; Index < BufferComponent->MaxBufferSize; ++Index)
	{
		const float SlotX = BufferMarginFromEdge + Index * (BufferIconSize + BufferIconSpacing);

		if (Buffered.IsValidIndex(Index) && Buffered[Index] && Buffered[Index]->Icon)
		{
			DrawTexture(Buffered[Index]->Icon, SlotX, StartY, BufferIconSize, BufferIconSize, 0.f, 0.f, 1.f, 1.f);
		}
		else
		{
			// Emplacement vide : simple fond sombre translucide pour marquer la place
			DrawRect(FLinearColor(0.2f, 0.2f, 0.2f, 0.5f), SlotX, StartY, BufferIconSize, BufferIconSize);
		}
	}
}

void AMageHUD::DrawDashCooldown(const APawn* Pawn)
{
	const AMageCharacter* MageCharacter = Cast<AMageCharacter>(Pawn);
	if (!MageCharacter || !Canvas)
	{
		return;
	}

	const float CooldownFraction = MageCharacter->GetDashCooldownRemainingFraction();
	const bool bIsReady = CooldownFraction <= 0.f;

	const UWorld* World = GetWorld();
	const double Now = World ? World->GetTimeSeconds() : 0.0;

	// Detecte la transition pret <-> en cooldown pour declencher un flash ponctuel (une seule fois
	// par transition, pas a chaque frame ou le dash reste dans le meme etat)
	if (bIsReady != bWasDashReadyLastFrame)
	{
		DashFlashStartTimeSeconds = Now;
		CurrentDashFlashColor = bIsReady ? DashReadyFlashColor : DashConsumedFlashColor;
	}
	bWasDashReadyLastFrame = bIsReady;

	const float X = DashMarginFromEdge;
	const float Y = DashMarginFromEdge;

	// Fond sombre : portion "pas encore disponible"
	DrawRect(FLinearColor(0.1f, 0.1f, 0.1f, 0.6f), X, Y, DashIconSize, DashIconSize);

	// Remplissage qui monte du bas vers le haut au fur et a mesure que le cooldown se termine
	const float FillHeight = DashIconSize * (1.f - CooldownFraction);
	if (FillHeight > 0.f)
	{
		DrawRect(FLinearColor(0.3f, 0.6f, 1.f, 0.9f), X, Y + (DashIconSize - FillHeight), DashIconSize, FillHeight);
	}

	// Contour de la case
	DrawLine(X, Y, X + DashIconSize, Y, FLinearColor::White, 1.f);
	DrawLine(X, Y, X, Y + DashIconSize, FLinearColor::White, 1.f);
	DrawLine(X + DashIconSize, Y, X + DashIconSize, Y + DashIconSize, FLinearColor::White, 1.f);
	DrawLine(X, Y + DashIconSize, X + DashIconSize, Y + DashIconSize, FLinearColor::White, 1.f);

	// Flash ponctuel (consommation ou retour de disponibilite), s'estompe avec le temps
	if (DashFlashStartTimeSeconds >= 0.0)
	{
		const float FlashElapsed = static_cast<float>(Now - DashFlashStartTimeSeconds);
		if (FlashElapsed < DashFlashDurationSeconds)
		{
			const float FlashAlpha = 1.f - (FlashElapsed / DashFlashDurationSeconds);
			FLinearColor FlashColor = CurrentDashFlashColor;
			FlashColor.A = FlashAlpha;
			DrawRect(FlashColor, X, Y, DashIconSize, DashIconSize);
		}
	}
}

void AMageHUD::DrawCrosshair(const FVector2D& Center)
{
	const float HalfLength = 8.f;
	const float Thickness = 2.f;

	DrawLine(Center.X - HalfLength, Center.Y, Center.X + HalfLength, Center.Y, FLinearColor::White, Thickness);
	DrawLine(Center.X, Center.Y - HalfLength, Center.X, Center.Y + HalfLength, FLinearColor::White, Thickness);
}
