// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemies/MageEnemyCharacter.h"

#include "Animation/AnimSequence.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Elements/ElementalStatusComponent.h"
#include "Elements/ElementFunctionLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Particles/ParticleSystemComponent.h"
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

	FireVFX = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("FireVFX"));
	FireVFX->SetupAttachment(GetMesh(), TEXT("FireSocket"));
	FireVFX->bAutoActivate = false;
	// P_Fire_Big a l'echelle 1 est enorme (couvre tout l'ecran a bout portant) -- reduit pour un feu
	// de personnage plutot qu'un brasier
	FireVFX->SetRelativeScale3D(FVector(0.35f));
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

	// Un vrai squelette a ete assigne a GetMesh() (ex: le Goblin) plutot que de rester sur le
	// PlaceholderMesh (capsule) -- applique la meme teinte dynamique dessus et cache la capsule pour
	// eviter un double mesh si quelqu'un a oublie de vider son Static Mesh sur ce Blueprint.
	if (GetMesh() && GetMesh()->GetSkeletalMeshAsset())
	{
		if (TintMaterial)
		{
			for (int32 SlotIndex = 0; SlotIndex < GetMesh()->GetNumMaterials(); ++SlotIndex)
			{
				GetMesh()->SetMaterial(SlotIndex, TintMaterial);
			}
		}

		PlaceholderMesh->SetVisibility(false);

		if (IdleAnimation)
		{
			GetMesh()->PlayAnimation(IdleAnimation, true);
		}
	}

	if (FireLoopVFXTemplate)
	{
		FireVFX->SetTemplate(FireLoopVFXTemplate);
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

	// VFX de feu -- independant de TintMaterial (deux reactions visuelles distinctes a NewElement,
	// l'une ne doit pas dependre de l'autre)
	if (NewElement == EElement::Fire)
	{
		if (FireVFX)
		{
			FireVFX->Activate(true);
		}

		if (IgniteBurstVFXTemplate)
		{
			UGameplayStatics::SpawnEmitterAttached(IgniteBurstVFXTemplate, GetMesh(), TEXT("FireSocket"));
		}
	}
	else if (FireVFX)
	{
		FireVFX->Deactivate();
	}

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
	case EElement::Lightning:
		Color = FLinearColor(1.0f, 0.9f, 0.2f);
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
		SetRunning(false);
		return;
	}

	FVector Direction = ToPlayer;
	Direction.Z = 0.f;
	if (Direction.Normalize())
	{
		SetRunning(true);
		AddMovementInput(Direction, 1.f);
	}
}

void AMageEnemyCharacter::SetRunning(bool bRunning)
{
	if (bRunning == bIsRunning || !GetMesh() || !GetMesh()->GetSkeletalMeshAsset())
	{
		return;
	}
	bIsRunning = bRunning;

	if (UAnimSequence* NextAnim = bRunning ? RunAnimation : IdleAnimation)
	{
		GetMesh()->PlayAnimation(NextAnim, true);
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
			UElementFunctionLibrary::DeliverElementToActor(ElementalStatus->GetCurrentElement(), ElementalStatus->GetCurrentSpellId(), Hit.GetActor(), this, Hit.Location);
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
	SetRunning(false);
}

void AMageEnemyCharacter::OnThrown_Implementation(AActor* Thrower, const FVector& ThrowVelocity)
{
	MotionState = EEnemyMotionState::Thrown;
	ThrownVelocity = ThrowVelocity.GetSafeNormal() * ThrownFlightSpeed;
}
