// Fill out your copyright notice in the Description page of Project Settings.


#include "Scavengers/Environment/Interactable.h"

// Add default functionality here for any IInteractable functions that are not pure virtual.

void IInteractable::Interact()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, __FUNCTION__);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "No code was written in the function");
}

void IInteractable::test()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, __FUNCTION__);
}