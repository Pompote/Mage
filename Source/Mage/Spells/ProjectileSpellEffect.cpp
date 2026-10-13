// Fill out your copyright notice in the Description page of Project Settings.

#include "Spells/ProjectileSpellEffect.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Spells/SpellDefinition.h"
#include "Spells/SpellProjectile.h"

void UProjectileSpellEffect::Execute_Implementation(const FSpellCastContext& Context)
{
	AActor* Caster = Context.Caster;
	if (!Caster || !ProjectileClass)
	{
		return;
	}

	const UCameraComponent* Camera = Caster->FindComponentByClass<UCameraComponent>();
	if (!Camera)
	{
		return;
	}

	const FVector SpawnLocation = Camera->GetComponentLocation() + Camera->GetForwardVector() * SpawnForwardOffset;
	const FRotator SpawnRotation = Camera->GetComponentRotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Caster;
	SpawnParams.Instigator = Cast<APawn>(Caster);
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (ASpellProjectile* Projectile = Caster->GetWorld()->SpawnActor<ASpellProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams))
	{
		Projectile->SetElement(Context.Element);

		if (Context.SpellDefinition)
		{
			Projectile->SetSpellId(Context.SpellDefinition->SpellId);
			Projectile->SetImprintsEnemy(Context.SpellDefinition->bImprintsEnemy);
			Projectile->SetPersistsOnAnySurface(Context.SpellDefinition->bPersistsOnAnySurface);
		}
	}
}
