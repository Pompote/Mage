// Fill out your copyright notice in the Description page of Project Settings.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "Spells/GestureShapeTemplate.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGestureShapeTemplateBakeTest, "Mage.Spells.GestureShapeTemplateBake", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FGestureShapeTemplateBakeTest::RunTest(const FString& Parameters)
{
	// OrderPointsGreedy : des points le long d'une ligne droite, melanges, doivent ressortir dans
	// un ordre monotone (le seul ordre "plus proche voisin" possible sur une ligne sans branche)
	{
		TArray<FVector2D> ShuffledLine;
		ShuffledLine.Add(FVector2D(40.0, 0.0));
		ShuffledLine.Add(FVector2D(0.0, 0.0));
		ShuffledLine.Add(FVector2D(100.0, 0.0));
		ShuffledLine.Add(FVector2D(20.0, 0.0));
		ShuffledLine.Add(FVector2D(80.0, 0.0));
		ShuffledLine.Add(FVector2D(60.0, 0.0));

		const TArray<FVector2D> Ordered = UGestureShapeTemplate::OrderPointsGreedy(ShuffledLine);

		TestEqual(TEXT("Le tri ne perd ni n'ajoute de points"), Ordered.Num(), ShuffledLine.Num());

		bool bIsMonotone = true;
		for (int32 Index = 1; Index < Ordered.Num(); ++Index)
		{
			if (Ordered[Index].X < Ordered[Index - 1].X)
			{
				bIsMonotone = false;
				break;
			}
		}
		TestTrue(TEXT("Des points sur une ligne droite doivent ressortir dans un ordre monotone en X"), bIsMonotone);
	}

	// DownsampleToGrid : un nuage dense de points doit se reduire, sans devenir vide
	{
		TArray<FVector2D> DensePoints;
		for (int32 X = 0; X < 20; ++X)
		{
			for (int32 Y = 0; Y < 20; ++Y)
			{
				DensePoints.Add(FVector2D(static_cast<double>(X), static_cast<double>(Y)));
			}
		}

		const TArray<FVector2D> Downsampled = UGestureShapeTemplate::DownsampleToGrid(DensePoints, 4.0);

		TestTrue(TEXT("Le sous-echantillonnage doit reduire le nombre de points"), Downsampled.Num() < DensePoints.Num());
		TestTrue(TEXT("Le sous-echantillonnage ne doit pas vider le nuage de points"), Downsampled.Num() > 0);
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
