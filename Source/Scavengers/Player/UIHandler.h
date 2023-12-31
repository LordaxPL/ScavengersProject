// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UIHandler.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SCAVENGERS_API UUIHandler : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UUIHandler();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// Widgets
	class UImage* HealthBar;
	class UImage* StaminaBar;
	class UImage* ProgressBar;
	class UImage* ProgressImage;
	class UImage* WeaponImages[3];
	class UUserWidget* UIWidget;
	UPROPERTY(EditAnywhere, category = "UI")
		TSubclassOf<UUserWidget> UIWidgetClass;

	// Variables
	class UMaterialInstanceDynamic* HealthBarMat;
	class UMaterialInstanceDynamic* StaminaBarMat;
	class UMaterialInstanceDynamic* ProgressBarMat_Dyn;
	class UMaterialInstance* ProgressBarMat;
	class UTexture2D* LockedTexture;
	class UTexture2D* UnlockedTexture;
	class APlayerCharacter* Player;
	float MaxHealth;
	float MaxStamina;
	float CurrentHealth;
	float CurrentStamina;
	float DisplayedHealth;
	float DisplayedStamina;
	float HealthChangeAmount;
	float StaminaChangeAmount;
	class UDataTable* ItemsTable;

	// Timing and smoothing
	FTimerHandle HealthBarLerpHandle;
	FTimerHandle StaminaBarLerpHandle;
	FTimerHandle NotificationShowHandle;
	FTimerHandle NotificationHideHandle;

	// Notification Widgets
	class UBorder* NotificationBorder;
	class UTextBlock* NotificationTextBlock;
	void FadeOutNotification();
	void FadeInNotification();
	void DelayNotificationFade();
	float CurrentNotificationOpacity;
	uint8 NotificationTimeToFadeOut;
	uint8 NotificationMaxDuration;

	// Lerping
	void LerpHealthBar();

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Functions
	// Version with lerp
	//void AdjustHealthBar(float HealthBefore, float HealthNow);
	// Version without lerp
	void AdjustHealthBar(float HealthAmount);
	void AdjustStaminaBar(float StaminaAmount);

	void ShowNotification(FString& NotificationText);

	// Sets the interaction image to LockedTexture, sets initial percentage to 0
	void ToggleProgressBar(bool bShow = true);

	// Sets the progress bar to display inProgress percent of completion
	void SetProgressBar(float inProgress);

	// sets the image visible during interaction
	// 1 = Locked
	// 2 = UnLocked
	void SwitchInteractionImage(uint8 ChosenImage);
	void SetWeaponSlotImage(uint8 SlotID, uint8 WeaponID);
	void SwitchWeapon(bool bUp = true);


};
