// Fill out your copyright notice in the Description page of Project Settings.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Spells/DollarOneRecognizer.h"

namespace DollarOneTestHelpers
{
	static TArray<FVector2D> MakeCircle(double Radius, int32 NumPoints)
	{
		TArray<FVector2D> Points;
		Points.Reserve(NumPoints);
		for (int32 Index = 0; Index < NumPoints; ++Index)
		{
			const double Angle = 2.0 * UE_DOUBLE_PI * Index / (NumPoints - 1);
			Points.Add(FVector2D(Radius * FMath::Cos(Angle), Radius * FMath::Sin(Angle)));
		}
		return Points;
	}

	static TArray<FVector2D> MakeSquareOutline(double Size, int32 PointsPerSide)
	{
		TArray<FVector2D> Points;
		const double H = Size * 0.5;
		auto AddEdge = [&Points, PointsPerSide](const FVector2D& From, const FVector2D& To)
		{
			for (int32 Index = 0; Index < PointsPerSide; ++Index)
			{
				const double Alpha = static_cast<double>(Index) / (PointsPerSide - 1);
				Points.Add(FMath::Lerp(From, To, Alpha));
			}
		};
		AddEdge(FVector2D(-H, -H), FVector2D(H, -H));
		AddEdge(FVector2D(H, -H), FVector2D(H, H));
		AddEdge(FVector2D(H, H), FVector2D(-H, H));
		AddEdge(FVector2D(-H, H), FVector2D(-H, -H));
		return Points;
	}

	static TArray<FVector2D> RotatePointsDegrees(const TArray<FVector2D>& Points, double Degrees)
	{
		const double Angle = FMath::DegreesToRadians(Degrees);
		const double CosTheta = FMath::Cos(Angle);
		const double SinTheta = FMath::Sin(Angle);

		TArray<FVector2D> Result;
		Result.Reserve(Points.Num());
		for (const FVector2D& Point : Points)
		{
			Result.Add(FVector2D(Point.X * CosTheta - Point.Y * SinTheta, Point.X * SinTheta + Point.Y * CosTheta));
		}
		return Result;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDollarOneRecognizerTest, "Mage.Spells.DollarOneRecognizer", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDollarOneRecognizerTest::RunTest(const FString& Parameters)
{
	using namespace DollarOneTestHelpers;

	UDollarOneRecognizer* Recognizer = NewObject<UDollarOneRecognizer>();

	FGestureTemplate CircleTemplate;
	CircleTemplate.Id = TEXT("Circle");
	CircleTemplate.NormalizedPoints = Recognizer->Normalize(MakeCircle(100.0, 64));

	FGestureTemplate SquareTemplate;
	SquareTemplate.Id = TEXT("Square");
	SquareTemplate.NormalizedPoints = Recognizer->Normalize(MakeSquareOutline(200.0, 16));

	const TArray<FGestureTemplate> Templates = { CircleTemplate, SquareTemplate };

	// Reconnaissance de base : un cercle doit matcher le template Cercle, pas Carre
	{
		const FGestureRecognitionResult Result = Recognizer->Recognize(MakeCircle(80.0, 40), Templates);
		TestEqual(TEXT("Un cercle doit reconnaitre le template Circle"), Result.TemplateId, FName(TEXT("Circle")));
		TestTrue(TEXT("Le score doit depasser le seuil de reconnaissance"), Result.bMatched);
	}

	// Reconnaissance de base : un carre doit matcher le template Carre, pas Cercle
	{
		const FGestureRecognitionResult Result = Recognizer->Recognize(MakeSquareOutline(150.0, 20), Templates);
		TestEqual(TEXT("Un carre doit reconnaitre le template Square"), Result.TemplateId, FName(TEXT("Square")));
		TestTrue(TEXT("Le score doit depasser le seuil de reconnaissance"), Result.bMatched);
	}

	// Invariance en rotation (dans la plage de recherche +/-45 deg) : un carre tourne de 20 deg
	// n'est pas symetrique a cet angle, donc ce test valide vraiment la recherche par section doree
	{
		const TArray<FVector2D> RotatedSquare = RotatePointsDegrees(MakeSquareOutline(180.0, 20), 20.0);
		const FGestureRecognitionResult Result = Recognizer->Recognize(RotatedSquare, Templates);
		TestEqual(TEXT("Un carre tourne de 20 degres doit toujours reconnaitre Square"), Result.TemplateId, FName(TEXT("Square")));
		TestTrue(TEXT("Le score doit rester au-dessus du seuil malgre la rotation"), Result.bMatched);
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
