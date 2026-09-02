// Fill out your copyright notice in the Description page of Project Settings.

#include "Elements/ElementReactionSubsystem.h"

#include "Elements/ElementReactionTable.h"
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
}

bool UElementReactionSubsystem::TryReact(EElement A, EElement B, FVector Location, AActor* SourceActor)
{
	if (!ReactionTable)
	{
		return false;
	}

	const FElementReactionRow* Row = ReactionTable->FindReaction(A, B);
	if (!Row)
	{
		UE_LOG(LogTemp, Warning, TEXT("ElementReactionSubsystem: aucune reaction trouvee pour %s + %s"),
			*UEnum::GetValueAsString(A), *UEnum::GetValueAsString(B));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("ElementReactionSubsystem: reaction '%s' (%s + %s) a %s"),
		*Row->ReactionId.ToString(), *UEnum::GetValueAsString(A), *UEnum::GetValueAsString(B), *Location.ToString());

	if (Row->ReactionEffectClass)
	{
		if (UWorld* World = GetWorld())
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			World->SpawnActor<AActor>(Row->ReactionEffectClass, Location, FRotator::ZeroRotator, SpawnParams);
		}
	}

	return true;
}
