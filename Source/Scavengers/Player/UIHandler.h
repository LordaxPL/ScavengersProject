// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UIHandler.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
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
	class UUserWidget* UIWidget;
	UPROPERTY(EditAnywhere, category = "UI")
	TSubclassOf<UUserWidget> UIWidgetClass;

	// Variables
	class UMaterialInstanceDynamic* HealthBarMat;
	class UMaterialInstanceDynamic* StaminaBarMat;
	class APlayerCharacter* Player;
	float MaxHealth;
	float MaxStamina;
	float CurrentHealth;
	float CurrentStamina;
	float DisplayedHealth;
	float DisplayedStamina;
	float HealthChangeAmount;
	float StaminaChangeAmount;

	// Timing and smoothing
	FTimerHandle HealthBarLerpHandle;
	FTimerHandle StaminaBarLerpHandle;
	FTimerDelegate HealthBarLerpDelegate;
	FTimerDelegate StaminaBarLerpDelegate;

	void LerpHealthBar();

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Functions
	// Version with lerp
	void AdjustHealthBar(float HealthBefore, float HealthNow);
	// Version without lerp
	void AdjustHealthBar(float HealthAmount);
	void AdjustStaminaBar(float StaminaAmount);

		
};
