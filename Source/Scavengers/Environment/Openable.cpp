// Fill out your copyright notice in the Description page of Project Settings.


#include "Scavengers/Environment/Openable.h"

// Sets default values
AOpenable::AOpenable()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SMObject(TEXT("StaticMesh'/Game/Assets/Exterior/SM_WindowOld.SM_WindowOld'"));
	if (SMObject.Succeeded())
	{
		Mesh->SetStaticMesh(SMObject.Object);
		Mesh->SetMobility(EComponentMobility::Static);
	}

	bIsOpen = false;
	LerpProgress = 0.0f;

}

// Called when the game starts or when spawned
void AOpenable::BeginPlay()
{
	Super::BeginPlay();
	OpenRotator = CloseRotator = GetActorRotation();
	bFlipDirection ? OpenRotator.Yaw += 90.0f : OpenRotator.Yaw -= 90.0f;
}

void AOpenable::Interact()
{
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Purple, "INTERACTING WITH AN OPENABLE");

	Mesh->SetMobility(EComponentMobility::Movable);
	//bIsOpen ? SetActorRotation(CloseRotator) : SetActorRotation(OpenRotator);
	//bIsOpen = !bIsOpen;
	bIsOpen ? Close() : Open();
}

void AOpenable::Open()
{
	if (!bIsOpen && !GetWorldTimerManager().IsTimerActive(OpenCloseHandle))
	{
		bIsLocked = false;
		bIsOpen = true;
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetWorldTimerManager().SetTimer(OpenCloseHandle, this, &AOpenable::LerpOpen, 0.03125f, true, 0.0f);
	}
}

void AOpenable::Close()
{
	if (bIsOpen && !GetWorldTimerManager().IsTimerActive(OpenCloseHandle))
	{
		bIsOpen = false;
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetWorldTimerManager().SetTimer(OpenCloseHandle, this, &AOpenable::LerpClose, 0.03125f, true, 0.0f);
	}
}

void AOpenable::LerpOpen()
{
	if (LerpProgress < 1.0f)
	{
		LerpProgress += LerpIntensity;
		SetActorRotation(FMath::LerpRange(CloseRotator, OpenRotator, LerpProgress));
	}
	else
	{
		LerpProgress = 1.0f;
		Mesh->SetMobility(EComponentMobility::Static);
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		GetWorldTimerManager().ClearTimer(OpenCloseHandle);
	}
}

void AOpenable::LerpClose()
{
	if (LerpProgress > 0.0f)
	{
		LerpProgress -= LerpIntensity;
		SetActorRotation(FMath::LerpRange(CloseRotator, OpenRotator, LerpProgress));
	}
	else
	{
		LerpProgress = 0.0f;
		Mesh->SetMobility(EComponentMobility::Static);
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		GetWorldTimerManager().ClearTimer(OpenCloseHandle);
	}
}

bool AOpenable::IsLocked() const
{
	return bIsLocked;
}