// Fill out your copyright notice in the Description page of Project Settings.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Spells/GestureShapeTemplate.h"
#include "Spells/DollarOneRecognizer.h"

namespace GestureShapeTemplateTestHelpers
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
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGestureShapeTemplateTest, "Mage.Spells.GestureShapeTemplate", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FGestureShapeTemplateTest::RunTest(const FString& Parameters)
{
	using namespace GestureShapeTemplateTestHelpers;

	UDollarOneRecognizer* Recognizer = NewObject<UDollarOneRecognizer>();

	UGestureShapeTemplate* UnbakedShape = NewObject<UGestureShapeTemplate>();
	TestFalse(TEXT("Un template fraichement cree sans points n'est pas bake"), UnbakedShape->IsBaked());

	// Simule ce que fera l'outil de bake (etape suivante) : normaliser une forme et la stocker sur l'asset
	UGestureShapeTemplate* CircleShape = NewObject<UGestureShapeTemplate>();
	CircleShape->ShapeId = TEXT("Circle");
	CircleShape->NormalizedPoints = Recognizer->Normalize(MakeCircle(100.0, 64));
	TestTrue(TEXT("Un template avec des points normalises est considere bake"), CircleShape->IsBaked());

	const FGestureTemplate Converted = CircleShape->ToGestureTemplate();
	TestEqual(TEXT("La conversion preserve l'ID de la forme"), Converted.Id, FName(TEXT("Circle")));
	TestEqual(TEXT("La conversion preserve le nombre de points"), Converted.NormalizedPoints.Num(), CircleShape->NormalizedPoints.Num());

	// Verifie que la structure convertie fonctionne reellement avec le recognizer, de bout en bout
	const TArray<FGestureTemplate> Templates = { Converted };
	const FGestureRecognitionResult Result = Recognizer->Recognize(MakeCircle(80.0, 40), Templates);
	TestEqual(TEXT("Un cercle dessine doit reconnaitre le UGestureShapeTemplate Circle converti"), Result.TemplateId, FName(TEXT("Circle")));
	TestTrue(TEXT("Le score doit depasser le seuil de reconnaissance"), Result.bMatched);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
