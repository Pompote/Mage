// Fill out your copyright notice in the Description page of Project Settings.

#include "Elements/ElementFunctionLibrary.h"

#include "Elements/ElementalStatusComponent.h"
#include "GameFramework/Actor.h"

bool UElementFunctionLibrary::DeliverElementToActor(EElement Element, FName SpellId, AActor* TargetActor, AActor* SourceActor, FVector ContactLocation)
{
	if (Element == EElement::None || !TargetActor)
	{
		return false;
	}

	UElementalStatusComponent* Status = TargetActor->FindComponentByClass<UElementalStatusComponent>();
	if (!Status)
	{
		UE_LOG(LogTemp, Warning, TEXT("ElementFunctionLibrary: DeliverElementToActor -- %s n'a pas de UElementalStatusComponent"), *TargetActor->GetName());
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("ElementFunctionLibrary: DeliverElementToActor -- %s (sort '%s') livre a %s"), *UEnum::GetValueAsString(Element), *SpellId.ToString(), *TargetActor->GetName());
	Status->ApplyElement(Element, SpellId, SourceActor, ContactLocation);
	return true;
}
