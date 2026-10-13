// Fill out your copyright notice in the Description page of Project Settings.

#include "Spells/PointCloudRecognizer.h"

double UPointCloudRecognizer::PathLength(const TArray<FVector2D>& Points)
{
	double Length = 0.0;
	for (int32 Index = 1; Index < Points.Num(); ++Index)
	{
		Length += FVector2D::Distance(Points[Index - 1], Points[Index]);
	}
	return Length;
}

TArray<FVector2D> UPointCloudRecognizer::Resample(const TArray<FVector2D>& Points, int32 N)
{
	TArray<FVector2D> Source = Points;
	if (Source.Num() < 2 || N < 2)
	{
		return Source;
	}

	const double Interval = PathLength(Source) / static_cast<double>(N - 1);
	double Accumulated = 0.0;

	TArray<FVector2D> Result;
	Result.Reserve(N);
	Result.Add(Source[0]);

	for (int32 Index = 1; Index < Source.Num(); ++Index)
	{
		const double SegmentLength = FVector2D::Distance(Source[Index - 1], Source[Index]);
		if (Accumulated + SegmentLength >= Interval)
		{
			const double Ratio = SegmentLength > UE_DOUBLE_KINDA_SMALL_NUMBER ? (Interval - Accumulated) / SegmentLength : 0.0;
			const FVector2D NewPoint = FMath::Lerp(Source[Index - 1], Source[Index], Ratio);
			Result.Add(NewPoint);
			Source.Insert(NewPoint, Index);
			Accumulated = 0.0;
		}
		else
		{
			Accumulated += SegmentLength;
		}
	}

	while (Result.Num() < N)
	{
		const FVector2D LastPoint = Result.Last();
		Result.Add(LastPoint);
	}
	if (Result.Num() > N)
	{
		Result.SetNum(N);
	}

	return Result;
}

FVector2D UPointCloudRecognizer::Centroid(const TArray<FVector2D>& Points)
{
	if (Points.Num() == 0)
	{
		return FVector2D::ZeroVector;
	}

	FVector2D Sum(0.0, 0.0);
	for (const FVector2D& Point : Points)
	{
		Sum += Point;
	}
	return Sum / static_cast<double>(Points.Num());
}

TArray<FVector2D> UPointCloudRecognizer::RotateBy(const TArray<FVector2D>& Points, double Radians)
{
	const FVector2D Center = Centroid(Points);
	const double CosTheta = FMath::Cos(Radians);
	const double SinTheta = FMath::Sin(Radians);

	TArray<FVector2D> Result;
	Result.Reserve(Points.Num());
	for (const FVector2D& Point : Points)
	{
		const FVector2D Delta = Point - Center;
		const double NewX = Delta.X * CosTheta - Delta.Y * SinTheta;
		const double NewY = Delta.X * SinTheta + Delta.Y * CosTheta;
		Result.Add(Center + FVector2D(NewX, NewY));
	}
	return Result;
}

double UPointCloudRecognizer::IndicativeAngle(const TArray<FVector2D>& Points)
{
	const FVector2D Center = Centroid(Points);
	return FMath::Atan2(Center.Y - Points[0].Y, Center.X - Points[0].X);
}

TArray<FVector2D> UPointCloudRecognizer::ScaleToSquareUniform(const TArray<FVector2D>& Points, double Size)
{
	double MinX = TNumericLimits<double>::Max();
	double MinY = TNumericLimits<double>::Max();
	double MaxX = TNumericLimits<double>::Lowest();
	double MaxY = TNumericLimits<double>::Lowest();

	for (const FVector2D& Point : Points)
	{
		MinX = FMath::Min(MinX, Point.X);
		MinY = FMath::Min(MinY, Point.Y);
		MaxX = FMath::Max(MaxX, Point.X);
		MaxY = FMath::Max(MaxY, Point.Y);
	}

	const double Width = FMath::Max(MaxX - MinX, UE_DOUBLE_KINDA_SMALL_NUMBER);
	const double Height = FMath::Max(MaxY - MinY, UE_DOUBLE_KINDA_SMALL_NUMBER);
	const double Scale = Size / FMath::Max(Width, Height);

	TArray<FVector2D> Result;
	Result.Reserve(Points.Num());
	for (const FVector2D& Point : Points)
	{
		Result.Add(Point * Scale);
	}
	return Result;
}

TArray<FVector2D> UPointCloudRecognizer::TranslateToOrigin(const TArray<FVector2D>& Points)
{
	const FVector2D Center = Centroid(Points);
	TArray<FVector2D> Result;
	Result.Reserve(Points.Num());
	for (const FVector2D& Point : Points)
	{
		Result.Add(Point - Center);
	}
	return Result;
}

double UPointCloudRecognizer::CloudDistance(const TArray<FVector2D>& PointsA, const TArray<FVector2D>& PointsB, int32 N, int32 Start)
{
	TArray<bool> Matched;
	Matched.Init(false, N);

	double Total = 0.0;
	int32 Index = Start;
	do
	{
		int32 BestIndex = -1;
		double BestDistSq = TNumericLimits<double>::Max();
		for (int32 Candidate = 0; Candidate < N; ++Candidate)
		{
			if (Matched[Candidate])
			{
				continue;
			}
			const double DistSq = FVector2D::DistSquared(PointsA[Index], PointsB[Candidate]);
			if (DistSq < BestDistSq)
			{
				BestDistSq = DistSq;
				BestIndex = Candidate;
			}
		}

		Matched[BestIndex] = true;
		const double Weight = 1.0 - (static_cast<double>((Index - Start + N) % N) / static_cast<double>(N));
		Total += Weight * FMath::Sqrt(BestDistSq);

		Index = (Index + 1) % N;
	} while (Index != Start);

	return Total;
}

double UPointCloudRecognizer::GreedyCloudMatch(const TArray<FVector2D>& PointsA, const TArray<FVector2D>& PointsB, int32 N)
{
	constexpr double Epsilon = 0.5;
	const int32 Step = FMath::Max(1, static_cast<int32>(FMath::Pow(static_cast<double>(N), 1.0 - Epsilon)));

	double MinDistance = TNumericLimits<double>::Max();
	for (int32 Start = 0; Start < N; Start += Step)
	{
		const double DistanceAToB = CloudDistance(PointsA, PointsB, N, Start);
		const double DistanceBToA = CloudDistance(PointsB, PointsA, N, Start);
		MinDistance = FMath::Min(MinDistance, FMath::Min(DistanceAToB, DistanceBToA));
	}

	// Moyenne par point plutot que la somme ponderee brute -- comparable a l'echelle d'une distance
	// point-a-point classique (voir Score dans Recognize)
	return MinDistance / static_cast<double>(N);
}

TArray<FVector2D> UPointCloudRecognizer::Normalize(const TArray<FVector2D>& RawPoints) const
{
	if (RawPoints.Num() < 2)
	{
		return RawPoints;
	}

	TArray<FVector2D> Points = Resample(RawPoints, NumResamplePoints);
	const double Angle = IndicativeAngle(Points);
	Points = RotateBy(Points, -Angle);
	Points = ScaleToSquareUniform(Points, SquareSize);
	Points = TranslateToOrigin(Points);
	return Points;
}

FGestureRecognitionResult UPointCloudRecognizer::Recognize(const TArray<FVector2D>& RawStroke, const TArray<FGestureTemplate>& Templates) const
{
	FGestureRecognitionResult Result;

	if (RawStroke.Num() < 2 || Templates.Num() == 0)
	{
		return Result;
	}

	const TArray<FVector2D> Candidate = Normalize(RawStroke);
	const double HalfDiagonal = 0.5 * FMath::Sqrt(2.0) * static_cast<double>(SquareSize);

	double BestDistance = TNumericLimits<double>::Max();
	const FGestureTemplate* BestTemplate = nullptr;

	for (const FGestureTemplate& Template : Templates)
	{
		const double Distance = GreedyCloudMatch(Candidate, Template.NormalizedPoints, NumResamplePoints);
		if (Distance < BestDistance)
		{
			BestDistance = Distance;
			BestTemplate = &Template;
		}
	}

	const double Score = 1.0 - (BestDistance / HalfDiagonal);

	double ThresholdToUse = static_cast<double>(RecognitionThreshold);
	if (BestTemplate && BestTemplate->RecognitionThresholdOverride > 0.f)
	{
		ThresholdToUse = static_cast<double>(BestTemplate->RecognitionThresholdOverride);
	}

	Result.TemplateId = BestTemplate ? BestTemplate->Id : NAME_None;
	Result.Score = static_cast<float>(Score);
	Result.bMatched = Score >= ThresholdToUse;

	return Result;
}
