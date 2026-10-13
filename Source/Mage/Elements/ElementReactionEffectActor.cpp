// Fill out your copyright notice in the Description page of Project Settings.

#include "Elements/ElementReactionEffectActor.h"

#include "DrawDebugHelpers.h"

AElementReactionEffectActor::AElementReactionEffectActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AElementReactionEffectActor::BeginPlay()
{
	Super::BeginPlay();

	DrawDebugSphere(GetWorld(), GetActorLocation(), 50.f, 12, DebugColor, false, LifetimeSeconds);
	SetLifeSpan(LifetimeSeconds);
}

void AElementReactionEffectActor::Setup_Implementation(AActor* TargetActor, AActor* SourceActor, EElement ElementA, EElement ElementB)
{
}
