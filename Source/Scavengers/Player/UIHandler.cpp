// Fill out your copyright notice in the Description page of Project Settings.


#include "UIHandler.h"
#include "PlayerCharacter.h"
#include "Components/Image.h"
#include "Components/WrapBox.h"
#include "Components/Border.h"
#include "Components/CanvasPanelSlot.h"
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
	
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> ProgressBarMatObject(TEXT("MaterialInstanceConstant'/Game/Materials/UIMaterials/MI_ProgressBar.MI_ProgressBar'"));
	if (ProgressBarMatObject.Succeeded())
	{
		ProgressBarMat = ProgressBarMatObject.Object;
	}

	static ConstructorHelpers::FObjectFinder<UTexture2D> LockedTextureObject(TEXT("Texture2D'/Game/Textures/UITextures/T_Locked.T_Locked'"));
	if (LockedTextureObject.Succeeded())
	{
		LockedTexture = LockedTextureObject.Object;
	}

	static ConstructorHelpers::FObjectFinder<UTexture2D> UnlockedTextureObject(TEXT("Texture2D'/Game/Textures/UITextures/T_Unlocked.T_Unlocked'"));
	if (UnlockedTextureObject.Succeeded())
	{
		UnlockedTexture = UnlockedTextureObject.Object;
	}
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

void UUIHandler::ToggleProgressBar(bool bShow)
{
	if (bShow)
	{
		UPanelWidget* rootPanel = (UPanelWidget*)UIWidget->GetRootWidget();
		FVector2D ProgressBarSize(250.0f);
		FVector2D ProgressImageSize(100.0f);

		if (ProgressBar == nullptr)
		{
			ProgressBar = NewObject<UImage>(UImage::StaticClass());
			ProgressImage = NewObject<UImage>(UImage::StaticClass());

			if (rootPanel)
			{
				rootPanel->AddChild(ProgressBar);
				rootPanel->AddChild(ProgressImage);

				ProgressBar->SetBrushSize(ProgressBarSize);
				ProgressImage->SetBrushSize(ProgressImageSize);

				ProgressBar->SetBrushFromMaterial(ProgressBarMat);
				ProgressBarMat_Dyn = ProgressBar->GetDynamicMaterial();

				ProgressBar->GetDynamicMaterial()->SetScalarParameterValue(FName("Percentage"), 0.0f);

			}
		}
		else
		{
			if (rootPanel && ProgressBar && ProgressImage)
			{
				rootPanel->AddChild(ProgressBar);
				rootPanel->AddChild(ProgressImage);
			}
		}

		UCanvasPanelSlot* ProgressBarSlot = (UCanvasPanelSlot*)ProgressBar->Slot;
		UCanvasPanelSlot* ProgressImageSlot = (UCanvasPanelSlot*)ProgressImage->Slot;

		ProgressBarSlot->SetSize(ProgressBarSize);
		ProgressImageSlot->SetSize(ProgressImageSize);

		ProgressBarSlot->SetAnchors(FAnchors(0.5f));
		ProgressImageSlot->SetAnchors(FAnchors(0.5f));

		ProgressBarSlot->SetPosition(ProgressBarSize * -0.5f);
		ProgressImageSlot->SetPosition(ProgressImageSize * -0.5f);

		ProgressImage->SetBrushFromTexture(LockedTexture);

	}
	else
	{
		if (ProgressBar)
		{
			ProgressBar->RemoveFromParent();
			ProgressImage->RemoveFromParent();
			ProgressImage->SetBrushFromTexture(LockedTexture);
			ProgressBarMat_Dyn->SetScalarParameterValue(FName("Percentage"), 0.0f);
		}
	}
}

void UUIHandler::SetProgressBar(float inProgress)
{
	if (ProgressBarMat_Dyn)
	{
		ProgressBarMat_Dyn->SetScalarParameterValue(FName("Percentage"), inProgress);
	}
}

void UUIHandler::SwitchInteractionImage(uint8 ChosenImage)
{
	switch (ChosenImage)
	{
	case 1:
	default:
		ProgressImage->SetBrushFromTexture(LockedTexture);
		break;
	case 2:
		ProgressImage->SetBrushFromTexture(UnlockedTexture);
		break;
	}
}