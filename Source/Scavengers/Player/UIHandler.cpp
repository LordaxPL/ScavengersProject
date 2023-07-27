// Fill out your copyright notice in the Description page of Project Settings.


#include "UIHandler.h"
#include "PlayerCharacter.h"
#include "Components/Image.h"
#include "Blueprint/UserWidget.h"
#define Print(String) GEngine->AddOnScreenDebugMessage(-1,1.f,FColor::Green,String);

// Sets default values for this component's properties
UUIHandler::UUIHandler()
{
	PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void UUIHandler::BeginPlay()
{
	Super::BeginPlay();

	// Getting max health
	// We don't get Health because it may change. Max health doesn't change
	Player = Cast<APlayerCharacter>(GetOwner());
	MaxHealth = Player->GetMaxHealth();
	MaxStamina = Player->GetMaxStamina();
	DisplayedHealth = MaxHealth;
	DisplayedStamina = MaxStamina;
	HealthChangeAmount = MaxHealth * 0.01f;
	StaminaChangeAmount = MaxStamina * 0.01f;


	if (IsValid(UIWidgetClass))
	{
		UIWidget = CreateWidget(GetWorld(), UIWidgetClass);
		if (UIWidget != nullptr)
		{
			UIWidget->AddToViewport();
			HealthBar = Cast<UImage>(UIWidget->GetWidgetFromName(FName("HealthBarImage")));
			StaminaBar = Cast<UImage>(UIWidget->GetWidgetFromName(FName("StaminaBarImage")));

			if (HealthBar != nullptr)
			{
				HealthBarMat = HealthBar->GetDynamicMaterial();
				if (HealthBarMat != nullptr)
				{
					AdjustHealthBar(MaxHealth);
				}
			}

			if (StaminaBar != nullptr)
			{
				StaminaBarMat = StaminaBar->GetDynamicMaterial();
				if (StaminaBarMat != nullptr)
				{

					AdjustStaminaBar(MaxStamina);
				}
			}
		}
	}
	
}

// Called every frame
void UUIHandler::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UUIHandler::AdjustHealthBar(float HealthAmount)
{
	CurrentHealth = HealthAmount;
	if (!GetWorld()->GetTimerManager().IsTimerActive(HealthBarLerpHandle))
	{
		GetWorld()->GetTimerManager().SetTimer(HealthBarLerpHandle, this, &UUIHandler::LerpHealthBar, 0.05f, true);
	}
}

void UUIHandler::AdjustStaminaBar(float StaminaAmount)
{
	float Percent = StaminaAmount / MaxStamina;
	if (StaminaBarMat != nullptr)
	{
		StaminaBarMat->SetScalarParameterValue(FName("Percentage"), Percent);
	}

}

void UUIHandler::LerpHealthBar()
{
	bool bLoop = true;
	if (DisplayedHealth < CurrentHealth - 1.0f)
	{
		DisplayedHealth += HealthChangeAmount;
	}
	else if (DisplayedHealth > CurrentHealth + 1.0f)
	{
		DisplayedHealth -= HealthChangeAmount;
	}
	else
	{
		DisplayedHealth = CurrentHealth;
		bLoop = false;
	}
	float Percent = DisplayedHealth / MaxHealth;
	HealthBarMat->SetScalarParameterValue(FName("Percentage"), Percent);

	if (!bLoop)
	{
		GetWorld()->GetTimerManager().ClearTimer(HealthBarLerpHandle);
	}
}
