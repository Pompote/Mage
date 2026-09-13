// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemies/MageEnemyCharacter.h"

#include "Components/StaticMeshComponent.h"
#include "Elements/ElementalStatusComponent.h"
#include "Elements/ElementFunctionLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AMageEnemyCharacter::AMageEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCharacterMovement()->bOrientRotationToMovement = true;

	PlaceholderMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaceholderMesh"));
	PlaceholderMesh->SetupAttachment(RootComponent);
	PlaceholderMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CapsuleMeshAsset(TEXT("/Engine/BasicShapes/Capsule.Capsule"));
	if (CapsuleMeshAsset.Succeeded())
	{
		PlaceholderMesh->SetStaticMesh(CapsuleMeshAsset.Object);
	}

	ElementalStatus = CreateDefaultSubobject<UElementalStatusComponent>(TEXT("ElementalStatus"));
}

void AMageEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	if (BaseTintMaterial)
	{
		TintMaterial = UMaterialInstanceDynamic::Create(BaseTintMaterial, this);
		PlaceholderMesh->SetMaterial(0, TintMaterial);

		UE_LOG(LogTemp, Log, TEXT("MageEnemyCharacter: TintMaterial cree a partir de '%s' (TintMaterial=%s)"),
			*BaseTintMaterial->GetName(), TintMaterial ? TEXT("ok") : TEXT("NULL"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MageEnemyCharacter: BaseTintMaterial non assigne dans l'editeur -- pas de teinte possible"));
	}

	if (ElementalStatus)
	{
		ElementalStatus->OnElementChanged.AddDynamic(this, &AMageEnemyCharacter::HandleElementChanged);
	}
}

void AMageEnemyCharacter::HandleElementChanged(EElement NewElement)
{
	UE_LOG(LogTemp, Log, TEXT("MageEnemyCharacter: HandleElementChanged(%s) sur %s, TintMaterial=%s"),
		*UEnum::GetValueAsString(NewElement), *GetName(), TintMaterial ? TEXT("ok") : TEXT("NULL"));

	if (!TintMaterial)
	{
		return;
	}

	FLinearColor Color;
	switch (NewElement)
	{
	case EElement::Fire:
		Color = FLinearColor::Red;
		break;
	case EElement::Water:
		Color = FLinearColor::Blue;
		break;
	case EElement::Earth:
		Color = FLinearColor(0.4f, 0.25f, 0.1f);
		break;
	case EElement::Air:
		Color = FLinearColor(0.8f, 0.8f, 0.9f);
		break;
	default:
		Color = FLinearColor(0.3f, 0.3f, 0.3f);
		break;
	}

	TintMaterial->SetVectorParameterValue(TEXT("Color"), Color);
}

void AMageEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	switch (MotionState)
	{
	case EEnemyMotionState::Seeking:
		TickSeeking(DeltaTime);
		break;
	case EEnemyMotionState::Held:
		// Rien a faire ici -- UGrabThrowComponent repositionne cet acteur chaque frame tant qu'il
		// est tenu (voir GrabThrowComponent::TickComponent)
		break;
	case EEnemyMotionState::Thrown:
		TickThrown(DeltaTime);
		break;
	}
}

void AMageEnemyCharacter::TickSeeking(float DeltaTime)
{
	const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!PlayerPawn)
	{
		return;
	}

	const FVector ToPlayer = PlayerPawn->GetActorLocation() - GetActorLocation();
	if (ToPlayer.Size() <= StopDistance)
	{
		return;
	}

	FVector Direction = ToPlayer;
	Direction.Z = 0.f;
	if (Direction.Normalize())
	{
		AddMovementInput(Direction, 1.f);
	}
}

void AMageEnemyCharacter::TickThrown(float DeltaTime)
{
	// Vol gere a la main (voir commentaire de classe) -- on ne peut pas compter sur MOVE_Falling
	// pour la gravite ici, donc on l'integre nous-memes dans ThrownVelocity chaque frame.
	ThrownVelocity.Z += GetCharacterMovement()->GetGravityZ() * DeltaTime;

	FHitResult Hit;
	AddActorWorldOffset(ThrownVelocity * DeltaTime, true, &Hit);

	if (Hit.bBlockingHit && Hit.GetActor())
	{
		if (ElementalStatus)
		{
			UElementFunctionLibrary::DeliverElementToActor(ElementalStatus->GetCurrentElement(), Hit.GetActor(), this, Hit.Location);
			ElementalStatus->ClearElement();
		}

		MotionState = EEnemyMotionState::Seeking;
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}
}

bool AMageEnemyCharacter::CanBeGrabbed_Implementation() const
{
	return MotionState == EEnemyMotionState::Seeking;
}

void AMageEnemyCharacter::OnGrabbed_Implementation(AActor* Grabber)
{
	MotionState = EEnemyMotionState::Held;
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->SetMovementMode(MOVE_Flying);
}

void AMageEnemyCharacter::OnThrown_Implementation(AActor* Thrower, const FVector& ThrowVelocity)
{
	MotionState = EEnemyMotionState::Thrown;
	ThrownVelocity = ThrowVelocity.GetSafeNormal() * ThrownFlightSpeed;
}
