// Fill out your copyright notice in the Description page of Project Settings.

#include "Spells/SpellDrawComponent.h"

#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Spells/SpellRecognitionSubsystem.h"

USpellDrawComponent::USpellDrawComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USpellDrawComponent::BeginStroke()
{
	bIsDrawingStroke = true;
	PenPosition = FVector2D::ZeroVector;
	CurrentStroke.Reset();
	CurrentStroke.Add(PenPosition);
}

void USpellDrawComponent::AddStrokeInput(const FVector2D& Delta)
{
	if (!bIsDrawingStroke)
	{
		return;
	}

	PenPosition += Delta * StrokeSensitivity;
	CurrentStroke.Add(PenPosition);
}

void USpellDrawComponent::EndStroke()
{
	if (!bIsDrawingStroke)
	{
		return;
	}

	bIsDrawingStroke = false;

	if (const UWorld* EndWorld = GetWorld())
	{
		LastStrokeEndTimeSeconds = EndWorld->GetTimeSeconds();
	}

	if (CurrentStroke.Num() >= 2)
	{
		// Diagnostic temporaire : la densite du trace brut capture en jeu n'a encore jamais ete
		// inspectee (tous les diagnostics precedents portaient sur le bake, pas sur la capture live).
		double RawPathLength = 0.0;
		for (int32 Index = 1; Index < CurrentStroke.Num(); ++Index)
		{
			RawPathLength += FVector2D::Distance(CurrentStroke[Index - 1], CurrentStroke[Index]);
		}
		UE_LOG(LogTemp, Log, TEXT("USpellDrawComponent: trace brut capture -- %d points, longueur totale %.0f"),
			CurrentStroke.Num(), RawPathLength);

		const UWorld* World = GetWorld();
		const UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
		USpellRecognitionSubsystem* Subsystem = GameInstance ? GameInstance->GetSubsystem<USpellRecognitionSubsystem>() : nullptr;

		if (Subsystem)
		{
			const FGestureRecognitionResult Result = Subsystem->RecognizeStroke(CurrentStroke);

			if (GEngine)
			{
				const FString Message = Result.bMatched
					? FString::Printf(TEXT("Sort reconnu : %s (score %.2f)"), *Result.TemplateId.ToString(), Result.Score)
					: FString::Printf(TEXT("Dessin non reconnu (meilleur score %.2f)"), Result.Score);
				GEngine->AddOnScreenDebugMessage(-1, 3.f, Result.bMatched ? FColor::Green : FColor::Red, Message);
			}

			OnGestureRecognized.Broadcast(Result);
		}
	}

	CurrentStroke.Reset();
}

bool USpellDrawComponent::ShouldSuppressCameraInput() const
{
	if (bIsDrawingStroke)
	{
		return true;
	}

	if (LastStrokeEndTimeSeconds < 0.0)
	{
		return false;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	return (World->GetTimeSeconds() - LastStrokeEndTimeSeconds) < static_cast<double>(PostStrokeGraceSeconds);
}
