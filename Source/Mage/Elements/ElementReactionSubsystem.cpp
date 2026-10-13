// Fill out your copyright notice in the Description page of Project Settings.

#include "Elements/ElementReactionSubsystem.h"

#include "Elements/ElementReactionCarrierInterface.h"
#include "Elements/ElementReactionEffectActor.h"
#include "Elements/ElementReactionTable.h"
#include "Elements/SpellReactionTable.h"
#include "Engine/World.h"

void UElementReactionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (ReactionTableAssetPath.IsValid())
	{
		ReactionTable = Cast<UElementReactionTable>(ReactionTableAssetPath.TryLoad());
	}

	if (!ReactionTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("ElementReactionSubsystem: aucune UElementReactionTable chargee -- verifie ReactionTableAssetPath dans Config/DefaultGame.ini"));
	}

	if (SpellReactionTableAssetPath.IsValid())
	{
		SpellReactionTable = Cast<USpellReactionTable>(SpellReactionTableAssetPath.TryLoad());
	}
	// Pas de warning si absente : la table sort est un affinage optionnel au-dessus de la table
	// element (voir Doc/claude-plan.md) -- son absence est un etat normal tant qu'elle n'a pas encore
	// ete creee/renseignee.
}

bool UElementReactionSubsystem::TryReact(FName SpellIdA, EElement ElementA, FName SpellIdB, EElement ElementB,
	AActor* TargetActor, AActor* SourceActor, FVector Location,
	EElement& OutResultElement, float& OutIntensityMultiplier, bool& OutClearTargetStatus)
{
	OutResultElement = EElement::None;
	OutIntensityMultiplier = 1.f;
	OutClearTargetStatus = true;

	FName ReactionId;
	TSubclassOf<AActor> ReactionEffectClass;
	bool bFound = false;

	if (SpellReactionTable)
	{
		if (const FSpellReactionRow* Row = SpellReactionTable->FindReaction(SpellIdA, SpellIdB))
		{
			ReactionId = Row->ReactionId;
			ReactionEffectClass = Row->ReactionEffectClass;
			OutResultElement = Row->ResultElement;
			OutIntensityMultiplier = Row->IntensityMultiplier;
			OutClearTargetStatus = Row->bClearTargetStatus;
			bFound = true;
		}
	}

	if (!bFound && ReactionTable)
	{
		if (const FElementReactionRow* Row = ReactionTable->FindReaction(ElementA, ElementB))
		{
			ReactionId = Row->ReactionId;
			ReactionEffectClass = Row->ReactionEffectClass;
			OutResultElement = Row->ResultElement;
			OutIntensityMultiplier = Row->IntensityMultiplier;
			OutClearTargetStatus = Row->bClearTargetStatus;
			bFound = true;
		}
	}

	if (!bFound)
	{
		UE_LOG(LogTemp, Warning, TEXT("ElementReactionSubsystem: aucune reaction trouvee pour %s + %s"),
			*UEnum::GetValueAsString(ElementA), *UEnum::GetValueAsString(ElementB));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("ElementReactionSubsystem: reaction '%s' (%s + %s) a %s"),
		*ReactionId.ToString(), *UEnum::GetValueAsString(ElementA), *UEnum::GetValueAsString(ElementB), *Location.ToString());

	if (ReactionEffectClass)
	{
		if (UWorld* World = GetWorld())
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			if (AActor* Spawned = World->SpawnActor<AActor>(ReactionEffectClass, Location, FRotator::ZeroRotator, SpawnParams))
			{
				if (AElementReactionEffectActor* EffectActor = Cast<AElementReactionEffectActor>(Spawned))
				{
					EffectActor->Setup(TargetActor, SourceActor, ElementA, ElementB);
				}
			}
		}
	}

	if (SourceActor && SourceActor->GetClass()->ImplementsInterface(UElementReactionCarrier::StaticClass()))
	{
		IElementReactionCarrier::Execute_OnDeliveredElementReacted(SourceActor, ReactionId, TargetActor);
	}

	return true;
}
