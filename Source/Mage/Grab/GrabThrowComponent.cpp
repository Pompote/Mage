// Fill out your copyright notice in the Description page of Project Settings.

#include "Grab/GrabThrowComponent.h"

#include "Camera/CameraComponent.h"
#include "Grab/GrabbableInterface.h"
#include "GameFramework/Actor.h"

UGrabThrowComponent::UGrabThrowComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false; // rien a faire tant qu'aucun acteur n'est tenu
}

void UGrabThrowComponent::TryGrab()
{
	if (HeldActor.IsValid())
	{
		return;
	}

	AActor* Owner = GetOwner();
	const UCameraComponent* Camera = Owner ? Owner->FindComponentByClass<UCameraComponent>() : nullptr;
	if (!Camera)
	{
		return;
	}

	const FVector Start = Camera->GetComponentLocation();
	const FVector End = Start + Camera->GetForwardVector() * GrabRange;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);

	FHitResult Hit;
	const bool bHit = GetWorld()->SweepSingleByChannel(Hit, Start, End, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(GrabTraceRadius), Params);

	if (!bHit || !Hit.GetActor())
	{
		UE_LOG(LogTemp, Log, TEXT("GrabThrow: TryGrab -- rien touche a portee"));
		return;
	}

	AActor* Target = Hit.GetActor();
	if (!Target->Implements<UGrabbableInterface>() || !IGrabbableInterface::Execute_CanBeGrabbed(Target))
	{
		UE_LOG(LogTemp, Log, TEXT("GrabThrow: TryGrab -- %s n'est pas saisissable"), *Target->GetName());
		return;
	}

	HeldActor = Target;
	IGrabbableInterface::Execute_OnGrabbed(Target, Owner);
	SetComponentTickEnabled(true);

	UE_LOG(LogTemp, Log, TEXT("GrabThrow: %s saisi"), *Target->GetName());
}

void UGrabThrowComponent::ReleaseOrThrow()
{
	AActor* Held = HeldActor.Get();
	if (!Held)
	{
		return;
	}

	const AActor* Owner = GetOwner();
	const UCameraComponent* Camera = Owner ? Owner->FindComponentByClass<UCameraComponent>() : nullptr;
	const FVector Velocity = Camera ? Camera->GetForwardVector() * ThrowSpeed : FVector::ZeroVector;

	IGrabbableInterface::Execute_OnThrown(Held, GetOwner(), Velocity);

	UE_LOG(LogTemp, Log, TEXT("GrabThrow: %s lance"), *Held->GetName());

	HeldActor = nullptr;
	SetComponentTickEnabled(false);
}

void UGrabThrowComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AActor* Held = HeldActor.Get();
	if (!Held)
	{
		return;
	}

	const AActor* Owner = GetOwner();
	const UCameraComponent* Camera = Owner ? Owner->FindComponentByClass<UCameraComponent>() : nullptr;
	if (!Camera)
	{
		return;
	}

	const FVector CarryLocation = Camera->GetComponentLocation() + Camera->GetForwardVector() * CarryDistance;
	Held->SetActorLocation(CarryLocation, true);
}
