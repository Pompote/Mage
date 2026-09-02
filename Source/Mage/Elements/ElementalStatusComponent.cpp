// Fill out your copyright notice in the Description page of Project Settings.

#include "Elements/ElementalStatusComponent.h"

#include "Elements/ElementReactionSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "TimerManager.h"

UElementalStatusComponent::UElementalStatusComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UElementalStatusComponent::ApplyElement(EElement NewElement, AActor* SourceActor, FVector ContactLocation)
{
	if (NewElement == EElement::None)
	{
		return;
	}

	if (CurrentElement == EElement::None)
	{
		SetElementInternal(NewElement, GetDurationSeconds(NewElement));
		return;
	}

	if (CurrentElement == NewElement)
	{
		// Meme element : on rafraichit juste la duree, l'etat visible ne change pas -- pas de
		// broadcast pour ne pas redeclencher une teinte/VFX qui joue deja
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(ExpiryTimerHandle, this, &UElementalStatusComponent::HandleExpiry, GetDurationSeconds(NewElement), false);
		}
		return;
	}

	// Element different deja present : reaction -- resolue via UElementReactionSubsystem
	if (const UWorld* World = GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (UElementReactionSubsystem* ReactionSubsystem = GameInstance->GetSubsystem<UElementReactionSubsystem>())
			{
				const FVector Location = !ContactLocation.IsNearlyZero()
					? ContactLocation
					: (GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector);

				ReactionSubsystem->TryReact(CurrentElement, NewElement, Location, SourceActor);
			}
		}
	}

	ClearElement();
}

float UElementalStatusComponent::GetDurationSeconds(EElement Element) const
{
	const FElementAffinityEntry Affinity = AffinityProfile ? AffinityProfile->GetAffinity(Element) : FElementAffinityEntry();
	return BaseElementDurationSeconds * Affinity.DurationMultiplier;
}

void UElementalStatusComponent::SetElementInternal(EElement NewElement, float DurationSeconds)
{
	CurrentElement = NewElement;

	UE_LOG(LogTemp, Log, TEXT("ElementalStatus: %s applique sur %s (duree %.1fs), OnElementChanged.IsBound=%s"),
		*UEnum::GetValueAsString(CurrentElement),
		GetOwner() ? *GetOwner()->GetName() : TEXT("NULL"),
		DurationSeconds,
		OnElementChanged.IsBound() ? TEXT("true") : TEXT("false"));

	OnElementChanged.Broadcast(CurrentElement);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(ExpiryTimerHandle, this, &UElementalStatusComponent::HandleExpiry, DurationSeconds, false);
	}
}

void UElementalStatusComponent::ClearElement()
{
	CurrentElement = EElement::None;
	OnElementChanged.Broadcast(CurrentElement);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ExpiryTimerHandle);
	}
}

void UElementalStatusComponent::HandleExpiry()
{
	ClearElement();
}
