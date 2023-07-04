// Fill out your copyright notice in the Description page of Project Settings.


#include "LivingBeing.h"

// Add default functionality here for any ILivingBeing functions that are not pure virtual.
void ILivingBeing::Die()
{
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, __FUNCTION__);
}