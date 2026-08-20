// Fill out your copyright notice in the Description page of Project Settings.

#include "MageCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Spells/SpellBufferComponent.h"
#include "Spells/SpellDrawComponent.h"
#include "UObject/ConstructorHelpers.h"

// Sets default values
AMageCharacter::AMageCharacter()
{
	// Set this character to call Tick() every frame.
	PrimaryActorTick.bCanEverTick = true;

	// Vue a la premiere personne : le corps suit le yaw de la camera, pas d'auto-orientation vers le mouvement
	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(RootComponent);
	FirstPersonCamera->SetRelativeLocation(FVector(0.f, 0.f, BaseEyeHeight));
	FirstPersonCamera->bUsePawnControlRotation = true;

	// Placeholder visuel : capsule du moteur (toujours disponible, meme sans contenu projet)
	PlaceholderMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaceholderMesh"));
	PlaceholderMesh->SetupAttachment(RootComponent);
	PlaceholderMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// Le proprietaire ne voit pas son propre corps (vue FPS), les autres joueurs le verront en coop
	PlaceholderMesh->SetOwnerNoSee(true);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CapsuleMeshAsset(TEXT("/Engine/BasicShapes/Capsule.Capsule"));
	if (CapsuleMeshAsset.Succeeded())
	{
		PlaceholderMesh->SetStaticMesh(CapsuleMeshAsset.Object);
	}

	SpellDrawComponent = CreateDefaultSubobject<USpellDrawComponent>(TEXT("SpellDrawComponent"));
	SpellBufferComponent = CreateDefaultSubobject<USpellBufferComponent>(TEXT("SpellBufferComponent"));
}

// Called when the game starts or when spawned
void AMageCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}
}

void AMageCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!SpellDrawComponent || !SpellDrawComponent->IsDrawingStroke() || !Controller)
	{
		return;
	}

	const UWorld* World = GetWorld();
	const double Now = World ? World->GetTimeSeconds() : 0.0;
	const double StrokeStartTime = SpellDrawComponent->GetStrokeStartTimeSeconds();
	const float ElapsedDrawSeconds = StrokeStartTime >= 0.0 ? static_cast<float>(Now - StrokeStartTime) : 0.f;

	// Tremblement qui grandit avec la duree du trace, plafonne
	const float ShakeAmplitude = FMath::Min(DrawShakeMaxAmplitude, DrawShakeGrowthPerSecond * ElapsedDrawSeconds);
	if (ShakeAmplitude > 0.f)
	{
		const float ShakePhase = static_cast<float>(Now) * DrawShakeFrequency;
		AddControllerYawInput(FMath::Sin(ShakePhase) * ShakeAmplitude);
		AddControllerPitchInput(FMath::Cos(ShakePhase * 1.3f) * ShakeAmplitude);
	}

	// Micro-mouvements subtils en directions aleatoires : une seule impulsion par secousse, au
	// rythme de DrawMicroJitterFrequency (pas a chaque frame), tenue jusqu'a la prochaine
	if (DrawMicroJitterAmplitude > 0.f && DrawMicroJitterFrequency > 0.f)
	{
		const double JitterInterval = 1.0 / static_cast<double>(DrawMicroJitterFrequency);
		if (Now - LastJitterUpdateTimeSeconds >= JitterInterval)
		{
			LastJitterUpdateTimeSeconds = Now;
			AddControllerYawInput(FMath::FRandRange(-DrawMicroJitterAmplitude, DrawMicroJitterAmplitude));
			AddControllerPitchInput(FMath::FRandRange(-DrawMicroJitterAmplitude, DrawMicroJitterAmplitude));
		}
	}
}

// Called to bind functionality to input
void AMageCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMageCharacter::Move);
		}
		if (LookAction)
		{
			EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMageCharacter::Look);
		}
		if (JumpAction)
		{
			EnhancedInput->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
			EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		}
		if (DrawAction && SpellDrawComponent)
		{
			EnhancedInput->BindAction(DrawAction, ETriggerEvent::Started, SpellDrawComponent.Get(), &USpellDrawComponent::BeginStroke);
			EnhancedInput->BindAction(DrawAction, ETriggerEvent::Completed, SpellDrawComponent.Get(), &USpellDrawComponent::EndStroke);
		}
		if (CastSlotAction)
		{
			EnhancedInput->BindAction(CastSlotAction, ETriggerEvent::Started, this, &AMageCharacter::CastSlot);
			UE_LOG(LogTemp, Log, TEXT("MageCharacter: CastSlotAction lie (%s)"), *CastSlotAction->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("MageCharacter: CastSlotAction est NULL -- rien n'est assigne dans le Blueprint enfant, les touches 1/2/3 ne feront jamais rien"));
		}
	}
}

void AMageCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AMageCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	// Pendant un dessin, le delta souris alimente le trace ET fait tourner la camera de facon tres
	// attenuee (pas un blocage total) -- garde la sensation que le mage regarde autour de lui
	// pendant qu'il trace, voir README "Mobilite pendant le dessin"
	if (SpellDrawComponent && SpellDrawComponent->IsDrawingStroke())
	{
		SpellDrawComponent->AddStrokeInput(LookAxisVector);

		if (Controller)
		{
			AddControllerYawInput(LookAxisVector.X * CameraAttenuationWhileDrawing);
			AddControllerPitchInput(LookAxisVector.Y * CameraAttenuationWhileDrawing);
		}
		return;
	}

	// Courte fenetre juste apres la fin du trace : coupure totale (pas d'attenuation), pour bien
	// absorber le mouvement souris residuel au relachement plutot qu'une histoire de sensation
	if (SpellDrawComponent && SpellDrawComponent->ShouldSuppressCameraInput())
	{
		return;
	}

	if (Controller)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AMageCharacter::CastSlot(const FInputActionValue& Value)
{
	// La touche pressee alimente cette meme Input Action avec une valeur differente (1.0/2.0/3.0)
	// via son modificateur Scalar dans l'IMC -- on retrouve l'index d'emplacement (0-based) a partir
	// de cette valeur.
	const float SlotNumber = Value.Get<float>();
	const int32 SlotIndex = FMath::RoundToInt(SlotNumber) - 1;

	UE_LOG(LogTemp, Log, TEXT("MageCharacter: CastSlotAction declenche -- valeur brute=%.2f -> SlotIndex=%d"), SlotNumber, SlotIndex);

	if (!SpellBufferComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("MageCharacter: CastSlot ignore -- SpellBufferComponent est null"));
		return;
	}

	SpellBufferComponent->CastSlot(SlotIndex);
}
