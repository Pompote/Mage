// Fill out your copyright notice in the Description page of Project Settings.

#include "Spells/GestureShapeBatchGenerator.h"

#if WITH_EDITOR

#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/Texture2D.h"
#include "Misc/PackageName.h"
#include "Spells/GestureShapeTemplate.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"

void UGestureShapeBatchGenerator::GenerateMissingShapeTemplates()
{
	const FString SourcePath = SourceImagesDirectory.Path;
	const FString TargetPath = TargetDirectory.Path;

	if (SourcePath.IsEmpty() || TargetPath.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("UGestureShapeBatchGenerator: renseigne SourceImagesDirectory et TargetDirectory avant de generer"));
		return;
	}

	const IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();

	// Textures disponibles dans le dossier source
	TArray<FAssetData> TextureAssets;
	AssetRegistry.GetAssetsByPath(FName(*SourcePath), TextureAssets, /*bRecursive=*/true);

	// Images deja utilisees par une forme existante dans le dossier cible
	TArray<FAssetData> ExistingShapeAssets;
	AssetRegistry.GetAssetsByPath(FName(*TargetPath), ExistingShapeAssets, /*bRecursive=*/true);

	TSet<FSoftObjectPath> UsedImages;
	for (const FAssetData& AssetData : ExistingShapeAssets)
	{
		if (const UGestureShapeTemplate* Existing = Cast<UGestureShapeTemplate>(AssetData.GetAsset()))
		{
			if (Existing->SourceImage)
			{
				UsedImages.Add(FSoftObjectPath(Existing->SourceImage));
			}
		}
	}

	int32 CreatedCount = 0;
	int32 SkippedCount = 0;

	for (const FAssetData& AssetData : TextureAssets)
	{
		UTexture2D* Texture = Cast<UTexture2D>(AssetData.GetAsset());
		if (!Texture)
		{
			continue;
		}

		if (UsedImages.Contains(FSoftObjectPath(Texture)))
		{
			++SkippedCount;
			continue;
		}

		// Deduit un nom de forme propre : retire un prefixe de nommage de texture s'il y en a un
		FString CoreName = Texture->GetName();
		if (CoreName.StartsWith(TEXT("GST_")))
		{
			CoreName = CoreName.RightChop(4);
		}
		else if (CoreName.StartsWith(TEXT("T_")))
		{
			CoreName = CoreName.RightChop(2);
		}

		const FString AssetName = FString::Printf(TEXT("GST_%s"), *CoreName);
		const FString PackageName = TargetPath / AssetName;

		UPackage* Package = CreatePackage(*PackageName);
		UGestureShapeTemplate* NewShape = NewObject<UGestureShapeTemplate>(Package, FName(*AssetName), RF_Public | RF_Standalone | RF_Transactional);
		NewShape->ShapeId = FName(*CoreName);
		NewShape->SourceImage = Texture;
		NewShape->BakeFromImage();

		FAssetRegistryModule::AssetCreated(NewShape);
		Package->MarkPackageDirty();

		const FString PackageFileName = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
		UPackage::SavePackage(Package, NewShape, *PackageFileName, SaveArgs);

		UE_LOG(LogTemp, Log, TEXT("UGestureShapeBatchGenerator: %s cree et bake depuis %s"), *AssetName, *Texture->GetName());
		++CreatedCount;
	}

	UE_LOG(LogTemp, Log, TEXT("UGestureShapeBatchGenerator: termine -- %d forme(s) creee(s), %d image(s) deja utilisee(s) ignoree(s)"),
		CreatedCount, SkippedCount);
}

#endif // WITH_EDITOR
