// Fill out your copyright notice in the Description page of Project Settings.


#include "UIHandler.h"
#include "PlayerCharacter.h"
#include "Components/Image.h"
#include "Blueprint/UserWidget.h"

// Sets default values for this component's properties
UUIHandler::UUIHandler()
{
	PrimaryComponentTick.bCanEverTick = true;



}


// Called when the game starts
void UUIHandler::BeginPlay()
{
	Super::BeginPlay();

	// Getting max health
	// We don't get Health because it may change. Max health doesn't change
	Player = Cast<APlayerCharacter>(GetOwner());
	MaxHealth = Player->GetMaxHealth();
	DisplayedHealth = MaxHealth;
	HealthChangeAmount = MaxHealth * 0.01f;


	if (IsValid(UIWidgetClass))
	{
		UIWidget = CreateWidget(GetWorld(), UIWidgetClass);
		if (UIWidget != nullptr)
		{
			UIWidget->AddToViewport();
			HealthBar = Cast<UImage>(UIWidget->GetWidgetFromName(FName("HealthBarImage")));

			if (HealthBar != nullptr)
			{
				HealthBarMat = HealthBar->GetDynamicMaterial();
				if (HealthBarMat != nullptr)
				{
					AdjustHealthBar(MaxHealth);
				}

				//TArray < FMaterialParameterInfo > Info1;

				//TArray<FGuid> Info2;

				//HealthBarMat->GetAllScalarParameterInfo(Info1, Info2);
				//for (int i = 0; i < Info1.Num(); i++)
				//{
				//	UE_LOG(LogTemp, Warning, TEXT("Name = %s = %d"), *Info1[6].Name.ToString(), Info1[6].Index);
				//}
			}
		}
	}
	
}

// Called every frame
void UUIHandler::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UUIHandler::AdjustHealthBar(float HealthBefore, float HealthNow)
{

}

void UUIHandler::AdjustHealthBar(float HealthAmount)
{
	CurrentHealth = HealthAmount;
	if (!GetWorld()->GetTimerManager().IsTimerActive(HealthBarLerpHandle))
	{
		GetWorld()->GetTimerManager().SetTimer(HealthBarLerpHandle, this, &UUIHandler::LerpHealthBar, 0.05f, true);
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
