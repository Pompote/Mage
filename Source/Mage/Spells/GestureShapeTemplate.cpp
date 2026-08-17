// Fill out your copyright notice in the Description page of Project Settings.

#include "Spells/GestureShapeTemplate.h"

FPrimaryAssetId UGestureShapeTemplate::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(TEXT("GestureShapeTemplate"), GetFName());
}

FGestureTemplate UGestureShapeTemplate::ToGestureTemplate() const
{
	FGestureTemplate Template;
	Template.Id = ShapeId;
	Template.NormalizedPoints = NormalizedPoints;
	return Template;
}
