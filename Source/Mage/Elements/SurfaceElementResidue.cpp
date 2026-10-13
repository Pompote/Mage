// Fill out your copyright notice in the Description page of Project Settings.

#include "Elements/SurfaceElementResidue.h"

#include "Components/StaticMeshComponent.h"
#include "Elements/ElementalStatusComponent.h"

ASurfaceElementResidue::ASurfaceElementResidue()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RootComponent = Mesh;

	ElementalStatus = CreateDefaultSubobject<UElementalStatusComponent>(TEXT("ElementalStatus"));
}

void ASurfaceElementResidue::ImprintElement(EElement Element, FName SpellId)
{
	if (!ElementalStatus)
	{
		return;
	}

	ElementalStatus->OnElementChanged.AddDynamic(this, &ASurfaceElementResidue::HandleElementChanged);
	ElementalStatus->ApplyElement(Element, SpellId, this);
}

void ASurfaceElementResidue::HandleElementChanged(EElement NewElement)
{
	if (NewElement == EElement::None)
	{
		Destroy();
	}
}
