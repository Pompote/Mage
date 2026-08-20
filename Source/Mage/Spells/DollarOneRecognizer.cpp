// Fill out your copyright notice in the Description page of Project Settings.

#include "Spells/DollarOneRecognizer.h"

double UDollarOneRecognizer::PathLength(const TArray<FVector2D>& Points)
{
	double Length = 0.0;
	for (int32 Index = 1; Index < Points.Num(); ++Index)
	{
		Length += FVector2D::Distance(Points[Index - 1], Points[Index]);
	}
	return Length;
}

TArray<FVector2D> UDollarOneRecognizer::CanonicalizeWindingOrder(const TArray<FVector2D>& Points)
{
	if (Points.Num() < 3)
	{
		return Points;
	}

	// Formule du lacet (shoelace) : signe positif = anti-horaire, negatif = horaire
	double SignedArea = 0.0;
	for (int32 Index = 0; Index < Points.Num(); ++Index)
	{
		const FVector2D& Current = Points[Index];
		const FVector2D& Next = Points[(Index + 1) % Points.Num()];
		SignedArea += (Current.X * Next.Y) - (Next.X * Current.Y);
	}

	if (SignedArea >= 0.0)
	{
		return Points;
	}

	TArray<FVector2D> Reversed;
	Reversed.Reserve(Points.Num());
	for (int32 Index = Points.Num() - 1; Index >= 0; --Index)
	{
		Reversed.Add(Points[Index]);
	}
	return Reversed;
}

TArray<FVector2D> UDollarOneRecognizer::Resample(const TArray<FVector2D>& Points, int32 N)
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
			// Reinsere le point cree pour que le reste de ce segment soit reconsidere a l'iteration suivante
			Source.Insert(NewPoint, Index);
			Accumulated = 0.0;
		}
		else
		{
			Accumulated += SegmentLength;
		}
	}

	// L'arrondi flottant laisse parfois le resultat un point court ou long par rapport a N.
	// Copie la valeur avant Add() : passer Result.Last() directement serait une reference
	// vers l'interieur du tableau, invalidee si Add() doit reallouer pour grandir.
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

FVector2D UDollarOneRecognizer::Centroid(const TArray<FVector2D>& Points)
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

TArray<FVector2D> UDollarOneRecognizer::RotateBy(const TArray<FVector2D>& Points, double Radians)
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

double UDollarOneRecognizer::IndicativeAngle(const TArray<FVector2D>& Points)
{
	const FVector2D Center = Centroid(Points);
	return FMath::Atan2(Center.Y - Points[0].Y, Center.X - Points[0].X);
}

TArray<FVector2D> UDollarOneRecognizer::ScaleToSquare(const TArray<FVector2D>& Points, double Size)
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

	TArray<FVector2D> Result;
	Result.Reserve(Points.Num());
	for (const FVector2D& Point : Points)
	{
		Result.Add(FVector2D(Point.X * (Size / Width), Point.Y * (Size / Height)));
	}
	return Result;
}

TArray<FVector2D> UDollarOneRecognizer::TranslateToOrigin(const TArray<FVector2D>& Points)
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

double UDollarOneRecognizer::PathDistance(const TArray<FVector2D>& PointsA, const TArray<FVector2D>& PointsB)
{
	// Les deux tableaux sont supposes de meme longueur (points deja reechantillonnes au meme N)
	const int32 Count = FMath::Min(PointsA.Num(), PointsB.Num());
	if (Count == 0)
	{
		return 0.0;
	}

	double Distance = 0.0;
	for (int32 Index = 0; Index < Count; ++Index)
	{
		Distance += FVector2D::Distance(PointsA[Index], PointsB[Index]);
	}
	return Distance / static_cast<double>(Count);
}

TArray<FVector2D> UDollarOneRecognizer::Normalize(const TArray<FVector2D>& RawPoints) const
{
	if (RawPoints.Num() < 2)
	{
		return RawPoints;
	}

	TArray<FVector2D> Points = Resample(RawPoints, NumResamplePoints);
	Points = CanonicalizeWindingOrder(Points);
	const double Angle = IndicativeAngle(Points);
	Points = RotateBy(Points, -Angle);
	Points = ScaleToSquare(Points, SquareSize);
	Points = TranslateToOrigin(Points);
	return Points;
}

double UDollarOneRecognizer::DistanceAtAngle(const TArray<FVector2D>& Points, const TArray<FVector2D>& TemplatePoints, double Radians) const
{
	const TArray<FVector2D> Rotated = RotateBy(Points, Radians);
	return PathDistance(Rotated, TemplatePoints);
}

double UDollarOneRecognizer::DistanceAtBestAngle(const TArray<FVector2D>& Points, const TArray<FVector2D>& TemplatePoints, double RadiansA, double RadiansB, double ThetaDeltaRadians) const
{
	// Recherche par section doree : converge vers l'angle de rotation qui minimise la distance
	// sans avoir a tester tous les angles un par un.
	static const double Phi = 0.5 * (-1.0 + FMath::Sqrt(5.0));

	double A = RadiansA;
	double B = RadiansB;
	double X1 = Phi * A + (1.0 - Phi) * B;
	double F1 = DistanceAtAngle(Points, TemplatePoints, X1);
	double X2 = (1.0 - Phi) * A + Phi * B;
	double F2 = DistanceAtAngle(Points, TemplatePoints, X2);

	while (FMath::Abs(B - A) > ThetaDeltaRadians)
	{
		if (F1 < F2)
		{
			B = X2;
			X2 = X1;
			F2 = F1;
			X1 = Phi * A + (1.0 - Phi) * B;
			F1 = DistanceAtAngle(Points, TemplatePoints, X1);
		}
		else
		{
			A = X1;
			X1 = X2;
			F1 = F2;
			X2 = (1.0 - Phi) * A + Phi * B;
			F2 = DistanceAtAngle(Points, TemplatePoints, X2);
		}
	}
	return FMath::Min(F1, F2);
}

FGestureRecognitionResult UDollarOneRecognizer::Recognize(const TArray<FVector2D>& RawStroke, const TArray<FGestureTemplate>& Templates) const
{
	FGestureRecognitionResult Result;

	if (RawStroke.Num() < 2 || Templates.Num() == 0)
	{
		return Result;
	}

	const TArray<FVector2D> Candidate = Normalize(RawStroke);

	const double ThetaRange = FMath::DegreesToRadians(static_cast<double>(ThetaRangeDegrees));
	const double ThetaDelta = FMath::DegreesToRadians(static_cast<double>(ThetaDeltaDegrees));
	const double HalfDiagonal = 0.5 * FMath::Sqrt(2.0) * static_cast<double>(SquareSize);

	double BestDistance = TNumericLimits<double>::Max();
	const FGestureTemplate* BestTemplate = nullptr;

	for (const FGestureTemplate& Template : Templates)
	{
		const double Distance = DistanceAtBestAngle(Candidate, Template.NormalizedPoints, -ThetaRange, ThetaRange, ThetaDelta);
		if (Distance < BestDistance)
		{
			BestDistance = Distance;
			BestTemplate = &Template;
		}
	}

	const double Score = 1.0 - (BestDistance / HalfDiagonal);

	// Le template gagnant peut imposer son propre seuil (ex: forme facilement confondue avec une
	// autre) ; <= 0 signifie qu'il n'en definit pas et que le seuil global du recognizer s'applique.
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
