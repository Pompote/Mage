// Fill out your copyright notice in the Description page of Project Settings.

#include "Spells/SpellBufferComponent.h"

#include "GameFramework/Actor.h"
#include "Spells/GestureRecognizer.h"
#include "Spells/GestureShapeTemplate.h"
#include "Spells/SpellCastContext.h"
#include "Spells/SpellDefinition.h"
#include "Spells/SpellDrawComponent.h"
#include "Spells/SpellEffect.h"

USpellBufferComponent::USpellBufferComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USpellBufferComponent::BeginPlay()
{
	Super::BeginPlay();

	const AActor* Owner = GetOwner();
	UE_LOG(LogTemp, Log, TEXT("SpellBuffer: BeginPlay sur %s -- KnownSpells=%d MaxBufferSize=%d"),
		Owner ? *Owner->GetName() : TEXT("NULL"), KnownSpells.Num(), MaxBufferSize);

	if (Owner)
	{
		if (USpellDrawComponent* DrawComponent = Owner->FindComponentByClass<USpellDrawComponent>())
		{
			DrawComponent->OnGestureRecognized.AddDynamic(this, &USpellBufferComponent::HandleGestureRecognized);
			UE_LOG(LogTemp, Log, TEXT("SpellBuffer: abonne a OnGestureRecognized de %s"), *DrawComponent->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("SpellBuffer: aucun SpellDrawComponent trouve sur %s -- le buffer ne recevra jamais de sort"), *Owner->GetName());
		}
	}
}

void USpellBufferComponent::HandleGestureRecognized(const FGestureRecognitionResult& Result)
{
	UE_LOG(LogTemp, Log, TEXT("SpellBuffer: geste recu -- TemplateId=%s Score=%.3f bMatched=%s"),
		*Result.TemplateId.ToString(), Result.Score, Result.bMatched ? TEXT("true") : TEXT("false"));

	if (!Result.bMatched)
	{
		return;
	}

	const TObjectPtr<USpellDefinition>* FoundSpell = KnownSpells.FindByPredicate([&Result](const TObjectPtr<USpellDefinition>& Spell)
	{
		return Spell && Spell->Shape && Spell->Shape->ShapeId == Result.TemplateId;
	});

	if (!FoundSpell || !*FoundSpell)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpellBuffer: aucun SpellDefinition dans KnownSpells (%d entree(s)) ne correspond a la forme '%s' -- verifie que KnownSpells est bien peuple et que le Shape assigne a un SpellDefinition a bien cet ShapeId"),
			KnownSpells.Num(), *Result.TemplateId.ToString());
		return;
	}

	if (BufferedSpells.Num() >= MaxBufferSize)
	{
		BufferedSpells.RemoveAt(0); // FIFO : le plus ancien sort du buffer
	}

	BufferedSpells.Add(*FoundSpell);
	UE_LOG(LogTemp, Log, TEXT("SpellBuffer: sort '%s' ajoute au buffer (%d/%d)"),
		*Result.TemplateId.ToString(), BufferedSpells.Num(), MaxBufferSize);
	OnBufferChanged.Broadcast();
}

void USpellBufferComponent::CastSlot(int32 SlotIndex)
{
	UE_LOG(LogTemp, Log, TEXT("SpellBuffer: CastSlot(%d) -- BufferedSpells.Num()=%d"), SlotIndex, BufferedSpells.Num());

	if (!BufferedSpells.IsValidIndex(SlotIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("SpellBuffer: CastSlot(%d) ignore -- index invalide ou emplacement vide"), SlotIndex);
		return;
	}

	if (USpellDefinition* Spell = BufferedSpells[SlotIndex])
	{
		if (Spell->EffectClass)
		{
			if (USpellEffect* Effect = NewObject<USpellEffect>(this, Spell->EffectClass))
			{
				FSpellCastContext Context;
				Context.Caster = GetOwner();
				Context.Element = Spell->Element;
				Context.SpellDefinition = Spell;
				Effect->Execute(Context);
			}
		}
	}

	BufferedSpells.RemoveAt(SlotIndex);
	UE_LOG(LogTemp, Log, TEXT("SpellBuffer: emplacement %d lance et retire du buffer"), SlotIndex);
	OnBufferChanged.Broadcast();
}
