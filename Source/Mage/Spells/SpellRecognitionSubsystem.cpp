// Fill out your copyright notice in the Description page of Project Settings.

#include "Spells/SpellRecognitionSubsystem.h"

#include "Engine/AssetManager.h"
#include "Spells/PointCloudRecognizer.h"
#include "Spells/GestureShapeTemplate.h"

void USpellRecognitionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Recognizer = NewObject<UPointCloudRecognizer>(this);
	LoadShapeTemplates();
}

void USpellRecognitionSubsystem::LoadShapeTemplates()
{
	LoadedTemplates.Reset();

	UAssetManager* AssetManager = UAssetManager::GetIfInitialized();
	if (!AssetManager)
	{
		return;
	}

	const FPrimaryAssetType ShapeType(TEXT("GestureShapeTemplate"));

	TArray<FPrimaryAssetId> ShapeIds;
	AssetManager->GetPrimaryAssetIdList(ShapeType, ShapeIds);

	if (ShapeIds.Num() == 0)
	{
		UE_LOG(LogTemp, Log, TEXT("USpellRecognitionSubsystem: aucune forme trouvee (dossier /Game/Spells/Shapes vide ou absent)"));
		return;
	}

	// Chargement synchrone : peu de Data Assets legers, acceptable au demarrage de la session.
	const TSharedPtr<FStreamableHandle> Handle = AssetManager->LoadPrimaryAssets(ShapeIds);
	if (Handle.IsValid())
	{
		Handle->WaitUntilComplete();
	}

	for (const FPrimaryAssetId& AssetId : ShapeIds)
	{
		const UGestureShapeTemplate* ShapeTemplate = Cast<UGestureShapeTemplate>(AssetManager->GetPrimaryAssetObject(AssetId));
		if (ShapeTemplate && ShapeTemplate->IsBaked())
		{
			LoadedTemplates.Add(ShapeTemplate->ToGestureTemplate());
		}
		else if (ShapeTemplate)
		{
			UE_LOG(LogTemp, Warning, TEXT("USpellRecognitionSubsystem: %s n'a pas ete bake, ignore"), *ShapeTemplate->GetName());
		}
	}

	UE_LOG(LogTemp, Log, TEXT("USpellRecognitionSubsystem: %d forme(s) chargee(s)"), LoadedTemplates.Num());
}

FGestureRecognitionResult USpellRecognitionSubsystem::RecognizeStroke(const TArray<FVector2D>& RawStroke) const
{
	if (!Recognizer)
	{
		return FGestureRecognitionResult();
	}

	return Recognizer->Recognize(RawStroke, LoadedTemplates);
}
