// Fill out your copyright notice in the Description page of Project Settings.


#include "UIHandler.h"
#include "PlayerCharacter.h"
#include "Components/Image.h"
#include "Components/WrapBox.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#define Print(String) GEngine->AddOnScreenDebugMessage(-1,1.f,FColor::Green,String);

// Sets default values for this component's properties
UUIHandler::UUIHandler()
{
	PrimaryComponentTick.bCanEverTick = false;
	NotificationMaxDuration = 3;
	NotificationTimeToFadeOut = NotificationMaxDuration;
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
			NotificationBorder = Cast<UBorder>(UIWidget->GetWidgetFromName(FName("NotificationBorder")));

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

			if (NotificationBorder != nullptr)
			{
				NotificationTextBlock = Cast<UTextBlock>(NotificationBorder->GetChildAt(0));
				if (NotificationTextBlock != nullptr)
				{
					GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, "Notification Text Block found");
					NotificationBorder->SetVisibility(ESlateVisibility::Collapsed);
					NotificationBorder->SetRenderOpacity(0.0f);
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


void UUIHandler::ShowNotification(FString& NotificationText)
{
	// If the notification is not visible
	if (NotificationBorder != nullptr)
	{
		if (NotificationBorder->GetVisibility() != ESlateVisibility::Visible)
		{
			NotificationBorder->SetRenderOpacity(0.0f);
			CurrentNotificationOpacity = 0.0f;
			GetWorld()->GetTimerManager().SetTimer(NotificationShowHandle, this, &UUIHandler::FadeInNotification, 0.1f, true);

		}

		// If the notification is pending to hide
		if (GetWorld()->GetTimerManager().IsTimerActive(NotificationHideHandle))
		{
			GetWorld()->GetTimerManager().ClearTimer(NotificationHideHandle);
			NotificationTimeToFadeOut = NotificationMaxDuration;
			GetWorld()->GetTimerManager().SetTimer(NotificationShowHandle, this, &UUIHandler::FadeInNotification, 0.1f, true);
		}

		NotificationTextBlock->SetText(FText::FromString(NotificationText));
	}
}

void UUIHandler::FadeOutNotification()
{
	if (CurrentNotificationOpacity > 0.0f)
	{
		NotificationBorder->SetRenderOpacity(CurrentNotificationOpacity);
		CurrentNotificationOpacity -= 0.1f;
	}
	else
	{
		NotificationBorder->SetRenderOpacity(0.0f);
		CurrentNotificationOpacity = 0.0f;
		NotificationBorder->SetVisibility(ESlateVisibility::Collapsed);
		GetWorld()->GetTimerManager().ClearTimer(NotificationShowHandle);
	}
}

void UUIHandler::FadeInNotification()
{
	if (NotificationBorder->GetVisibility() != ESlateVisibility::Visible)
	{
		NotificationBorder->SetVisibility(ESlateVisibility::Visible);
	}

	if (CurrentNotificationOpacity < 1.0f)
	{
		NotificationBorder->SetRenderOpacity(CurrentNotificationOpacity);
		CurrentNotificationOpacity += 0.1f;
	}
	else
	{
		NotificationBorder->SetRenderOpacity(1.0f);
		CurrentNotificationOpacity = 1.0f;
		GetWorld()->GetTimerManager().ClearTimer(NotificationShowHandle);
		GetWorld()->GetTimerManager().SetTimer(NotificationHideHandle, this, &UUIHandler::DelayNotificationFade, 1.0f, true);
	}
}

void UUIHandler::DelayNotificationFade()
{
	NotificationTimeToFadeOut--;
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Purple, FString::Printf(TEXT("Remaining time: %d"), NotificationTimeToFadeOut));
	if (NotificationTimeToFadeOut == 0)
	{
		NotificationTimeToFadeOut = NotificationMaxDuration;
		GetWorld()->GetTimerManager().ClearTimer(NotificationHideHandle);
		GetWorld()->GetTimerManager().SetTimer(NotificationHideHandle, this, &UUIHandler::FadeOutNotification, 0.1f, true);
	}
}

