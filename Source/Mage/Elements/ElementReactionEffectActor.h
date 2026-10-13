// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Elements/ElementTypes.h"
#include "GameFramework/Actor.h"
#include "ElementReactionEffectActor.generated.h"

// Placeholder generique pour le resultat visuel d'une reaction elementaire (Brume, Boue, etc.) --
// une sphere de debug qui s'efface au bout de LifetimeSeconds. A remplacer par un vrai mesh/Niagara
// par ligne de UElementReactionTable une fois qu'on aura de vrais assets (voir README "Mecanique #2").
UCLASS(Blueprintable)
class MAGE_API AElementReactionEffectActor : public AActor
{
	GENERATED_BODY()

public:
	AElementReactionEffectActor();

	// Appele juste apres le spawn par UElementReactionSubsystem::TryReact, avant BeginPlay -- donne
	// a un Blueprint enfant de quoi faire quelque chose de specifique a CETTE reaction (referencer
	// TargetActor/SourceActor, teinter selon ElementA/ElementB, etc.) plutot que juste dessiner une
	// sphere de debug. Vide par defaut : ne change rien au comportement actuel tant que rien ne le
	// surcharge. Voir Doc/claude-plan.md.
	UFUNCTION(BlueprintNativeEvent, Category = "Element")
	void Setup(AActor* TargetActor, AActor* SourceActor, EElement ElementA, EElement ElementB);
	virtual void Setup_Implementation(AActor* TargetActor, AActor* SourceActor, EElement ElementA, EElement ElementB);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Element")
	float LifetimeSeconds = 2.f;

	UPROPERTY(EditAnywhere, Category = "Element")
	FColor DebugColor = FColor::White;
};
