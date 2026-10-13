// Fill out your copyright notice in the Description page of Project Settings.

#include "Spells/GestureShapeTemplate.h"

#include "Spells/PointCloudRecognizer.h"

#if WITH_EDITOR
#include "Engine/Texture2D.h"
#endif

FPrimaryAssetId UGestureShapeTemplate::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(TEXT("GestureShapeTemplate"), GetFName());
}

FGestureTemplate UGestureShapeTemplate::ToGestureTemplate() const
{
	FGestureTemplate Template;
	Template.Id = ShapeId;
	Template.NormalizedPoints = NormalizedPoints;
	Template.RecognitionThresholdOverride = RecognitionThresholdOverride;
	return Template;
}

#if WITH_EDITOR

TArray<FVector2D> UGestureShapeTemplate::ExtractInkPointsFromImage() const
{
	TArray<FVector2D> InkPoints;

	if (!SourceImage)
	{
		UE_LOG(LogTemp, Warning, TEXT("UGestureShapeTemplate::ExtractInkPointsFromImage: %s n'a pas de SourceImage"), *GetName());
		return InkPoints;
	}

	FTextureSource& Source = SourceImage->Source;
	if (!Source.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("UGestureShapeTemplate::ExtractInkPointsFromImage: source art indisponible pour %s"), *SourceImage->GetName());
		return InkPoints;
	}

	if (Source.GetFormat() != TSF_BGRA8)
	{
		UE_LOG(LogTemp, Warning, TEXT("UGestureShapeTemplate::ExtractInkPointsFromImage: format non supporte pour %s (reimporter en BGRA8)"), *SourceImage->GetName());
		return InkPoints;
	}

	TArray64<uint8> RawData;
	if (!Source.GetMipData(RawData, 0))
	{
		UE_LOG(LogTemp, Warning, TEXT("UGestureShapeTemplate::ExtractInkPointsFromImage: lecture des pixels echouee pour %s"), *SourceImage->GetName());
		return InkPoints;
	}

	const int32 Width = Source.GetSizeX();
	const int32 Height = Source.GetSizeY();
	const uint8* Pixels = RawData.GetData();
	constexpr uint8 InkLuminanceThreshold = 128;
	constexpr uint8 MinOpacity = 10;

	for (int32 Y = 0; Y < Height; ++Y)
	{
		for (int32 X = 0; X < Width; ++X)
		{
			const int64 PixelIndex = (static_cast<int64>(Y) * Width + X) * 4;
			const uint8 B = Pixels[PixelIndex + 0];
			const uint8 G = Pixels[PixelIndex + 1];
			const uint8 R = Pixels[PixelIndex + 2];
			const uint8 A = Pixels[PixelIndex + 3];
			const uint8 Luminance = static_cast<uint8>((R + G + B) / 3);

			if (A > MinOpacity && Luminance < InkLuminanceThreshold)
			{
				// Y image vers le bas -> inverse pour un repere de dessin classique (Y vers le haut)
				InkPoints.Add(FVector2D(static_cast<double>(X), static_cast<double>(Height - Y)));
			}
		}
	}

	return InkPoints;
}

TArray<FVector2D> UGestureShapeTemplate::DownsampleToGrid(const TArray<FVector2D>& Points, double CellSize)
{
	TMap<TPair<int32, int32>, FVector2D> Cells;
	Cells.Reserve(Points.Num());

	for (const FVector2D& Point : Points)
	{
		const TPair<int32, int32> Cell(FMath::FloorToInt(Point.X / CellSize), FMath::FloorToInt(Point.Y / CellSize));
		if (!Cells.Contains(Cell))
		{
			Cells.Add(Cell, Point);
		}
	}

	TArray<FVector2D> Result;
	Cells.GenerateValueArray(Result);
	return Result;
}

TArray<FVector2D> UGestureShapeTemplate::OrderPointsGreedy(TArray<FVector2D> Points)
{
	TArray<FVector2D> Ordered;
	if (Points.Num() == 0)
	{
		return Ordered;
	}

	Ordered.Reserve(Points.Num());

	// Point de depart deterministe : le plus haut, puis le plus a gauche en cas d'egalite
	int32 StartIndex = 0;
	for (int32 Index = 1; Index < Points.Num(); ++Index)
	{
		if (Points[Index].Y > Points[StartIndex].Y ||
			(Points[Index].Y == Points[StartIndex].Y && Points[Index].X < Points[StartIndex].X))
		{
			StartIndex = Index;
		}
	}

	FVector2D Current = Points[StartIndex];
	Points.RemoveAtSwap(StartIndex);
	Ordered.Add(Current);

	while (Points.Num() > 0)
	{
		int32 NearestIndex = 0;
		double NearestDistSq = FVector2D::DistSquared(Current, Points[0]);
		for (int32 Index = 1; Index < Points.Num(); ++Index)
		{
			const double DistSq = FVector2D::DistSquared(Current, Points[Index]);
			if (DistSq < NearestDistSq)
			{
				NearestDistSq = DistSq;
				NearestIndex = Index;
			}
		}

		Current = Points[NearestIndex];
		Points.RemoveAtSwap(NearestIndex);
		Ordered.Add(Current);
	}

	return Ordered;
}

void UGestureShapeTemplate::BakeFromImage()
{
	TArray<FVector2D> InkPoints = ExtractInkPointsFromImage();
	if (InkPoints.Num() < 2)
	{
		UE_LOG(LogTemp, Warning, TEXT("UGestureShapeTemplate::BakeFromImage: pas assez de pixels d'encre trouves pour %s"), *GetName());
		return;
	}

	// Taille de cellule proportionnelle a la taille de la forme, pas un nombre de pixels fixe : un
	// trait epais a l'echelle d'une grande image a deux bords (interieur/exterieur) trop eloignes
	// pour fusionner dans la meme cellule avec une valeur fixe trop petite. Sans fusion, le tri
	// glouton epuise un bord avant de sauter sur l'autre et parcourt le contour deux fois au lieu
	// d'une (constate sur un cercle a haute resolution avec une cellule fixe de 4px).
	FVector2D MinPoint(TNumericLimits<double>::Max(), TNumericLimits<double>::Max());
	FVector2D MaxPoint(TNumericLimits<double>::Lowest(), TNumericLimits<double>::Lowest());
	for (const FVector2D& Point : InkPoints)
	{
		MinPoint.X = FMath::Min(MinPoint.X, Point.X);
		MinPoint.Y = FMath::Min(MinPoint.Y, Point.Y);
		MaxPoint.X = FMath::Max(MaxPoint.X, Point.X);
		MaxPoint.Y = FMath::Max(MaxPoint.Y, Point.Y);
	}
	const double Diagonal = FVector2D::Distance(MinPoint, MaxPoint);
	const double CellSize = FMath::Max(Diagonal / 64.0, 2.0);

	InkPoints = DownsampleToGrid(InkPoints, CellSize);
	InkPoints = OrderPointsGreedy(InkPoints);

	const UPointCloudRecognizer* TempRecognizer = NewObject<UPointCloudRecognizer>();
	NormalizedPoints = TempRecognizer->Normalize(InkPoints);

	MarkPackageDirty();

	UE_LOG(LogTemp, Log, TEXT("UGestureShapeTemplate::BakeFromImage: %s bake (cellule %.1fpx, %d points d'encre sous-echantillonnes -> %d points normalises)"),
		*GetName(), CellSize, InkPoints.Num(), NormalizedPoints.Num());
}

#endif // WITH_EDITOR
