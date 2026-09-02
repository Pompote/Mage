// Fill out your copyright notice in the Description page of Project Settings.

#include "Grab/GrabbableInterface.h"

bool IGrabbableInterface::CanBeGrabbed_Implementation() const
{
	return true;
}

void IGrabbableInterface::OnGrabbed_Implementation(AActor* Grabber)
{
}

void IGrabbableInterface::OnThrown_Implementation(AActor* Thrower, const FVector& ThrowVelocity)
{
}
