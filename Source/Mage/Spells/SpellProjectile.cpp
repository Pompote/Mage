// Fill out your copyright notice in the Description page of Project Settings.

#include "Spells/SpellProjectile.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Elements/ElementFunctionLibrary.h"
#include "GameFramework/ProjectileMovementComponent.h"

ASpellProjectile::ASpellProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->InitSphereRadius(15.f);
	// "Projectile" n'est pas un profil de collision reel (ni du moteur, ni de ce projet) --
	// SetCollisionProfileName echouait silencieusement avec ce nom, laissant la sphere sur son etat
	// de collision par defaut (probablement Overlap, pas Block -- d'ou les projectiles qui
	// traversaient tout sans jamais declencher OnHit). BlockAllDynamic est un vrai profil du moteur.
	CollisionComponent->BodyInstance.SetCollisionProfileName(TEXT("BlockAllDynamic"));
	CollisionComponent->BodyInstance.bNotifyRigidBodyCollision = true;
	CollisionComponent->OnComponentHit.AddDynamic(this, &ASpellProjectile::OnHit);
	RootComponent = CollisionComponent;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->InitialSpeed = 2000.f;
	ProjectileMovement->MaxSpeed = 2000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;

	InitialLifeSpan = 5.f;
}

void ASpellProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	UE_LOG(LogTemp, Log, TEXT("SpellProjectile: OnHit -- OtherActor=%s Element=%s"),
		OtherActor ? *OtherActor->GetName() : TEXT("NULL"), *UEnum::GetValueAsString(Element));

	if (Element != EElement::None && OtherActor)
	{
		UElementFunctionLibrary::DeliverElementToActor(Element, OtherActor, this, Hit.Location);
	}

	Destroy();
}
