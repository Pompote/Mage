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

void UElementalStatusComponent::ApplyElement(EElement NewElement, FName NewSpellId, AActor* SourceActor, FVector ContactLocation)
{
	if (NewElement == EElement::None)
	{
		return;
	}

	if (CurrentElement == EElement::None)
	{
		SetElementInternal(NewElement, NewSpellId, GetDurationSeconds(NewElement));
		return;
	}

	if (CurrentElement == NewElement)
	{
		// Meme element : on rafraichit juste la duree, l'etat visible ne change pas -- pas de
		// broadcast pour ne pas redeclencher une teinte/VFX qui joue deja. On retient quand meme
		// NewSpellId : la source la plus recente devient l'identite de reaction pertinente.
		CurrentSpellId = NewSpellId;
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(ExpiryTimerHandle, this, &UElementalStatusComponent::HandleExpiry, GetDurationSeconds(NewElement), false);
		}
		return;
	}

	// Element different deja present : reaction -- resolue via UElementReactionSubsystem, d'abord au
	// niveau du sort (CurrentSpellId/NewSpellId) puis, a defaut, au niveau de l'element (voir
	// Doc/claude-plan.md)
	bool bClearTargetStatus = true;
	EElement ResultElement = EElement::None;
	float IntensityMultiplier = 1.f;

	if (const UWorld* World = GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (UElementReactionSubsystem* ReactionSubsystem = GameInstance->GetSubsystem<UElementReactionSubsystem>())
			{
				const FVector Location = !ContactLocation.IsNearlyZero()
					? ContactLocation
					: (GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector);

				ReactionSubsystem->TryReact(CurrentSpellId, CurrentElement, NewSpellId, NewElement, GetOwner(), SourceActor, Location,
					ResultElement, IntensityMultiplier, bClearTargetStatus);
			}
		}
	}

	if (!bClearTargetStatus)
	{
		// Ligne de reaction "aucune interaction" -- le statut actuel reste intact
		return;
	}

	if (ResultElement != EElement::None)
	{
		// La reaction laisse un statut compose sur la cible (ex: "eau electrifiee") plutot que de
		// l'effacer -- ne vient plus directement d'un sort precis
		SetElementInternal(ResultElement, NAME_None, GetDurationSeconds(ResultElement) * IntensityMultiplier);
		return;
	}

	ClearElement();
}

float UElementalStatusComponent::GetDurationSeconds(EElement Element) const
{
	const FElementAffinityEntry Affinity = AffinityProfile ? AffinityProfile->GetAffinity(Element) : FElementAffinityEntry();
	return BaseElementDurationSeconds * Affinity.DurationMultiplier;
}

void UElementalStatusComponent::SetElementInternal(EElement NewElement, FName NewSpellId, float DurationSeconds)
{
	CurrentElement = NewElement;
	CurrentSpellId = NewSpellId;

	UE_LOG(LogTemp, Log, TEXT("ElementalStatus: %s (sort '%s') applique sur %s (duree %.1fs), OnElementChanged.IsBound=%s"),
		*UEnum::GetValueAsString(CurrentElement),
		*CurrentSpellId.ToString(),
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
	CurrentSpellId = NAME_None;
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
