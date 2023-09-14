// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Scavengers\ArtificialIntelligence\LivingBeing.h"


#include "PlayerCharacter.generated.h"

enum StatStatus
{
	Stable,
	Regenerating,
	Draining,
	Depleted
};

UCLASS()
class SCAVENGERS_API APlayerCharacter : public ACharacter, public ILivingBeing
{
	GENERATED_BODY()

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Camera and movement
	UPROPERTY(EditAnywhere)
	class UCameraComponent* PlayerCamera;
	UPROPERTY(EditAnywhere)
	class USpringArmComponent* CameraArm;
	FTimerHandle CameraLerpHandle;
	// zooms the camera
	void FocusCamera(bool bZoomIn);
	void LerpCameraZoomIn();
	void LerpCameraZoomOut();
	float ZoomProgress;

	// Movement
	void MoveForward(float Value);
	void MoveRight(float Value);
	void TurnLeftRight(float Value);
	void ToggleSprinting();
	void ToggleCrouching();
	bool CanUnCrouch();
	// Enables or disables player movement and rotation
	void TogglePlayerFreeze(bool bFreeze);
	bool bFreezed;
	class APlayerController* PlayerController;

	// Stamina (Stamina variable has already been declared in LivingBeing.h)
	FTimerHandle StaminaDelayHandle;
	StatStatus StaminaStatus;
	void InterruptStaminaRegeneration();
	void InterruptStaminaDraining();
	void InitStaminaRegeneration();
	void InitStaminaDraining();
	void RegenerateStamina();
	void DrainStamina();
	float StaminaRegenerationStrength;
	bool bIsSprinting;
	bool bCanSprint;
	bool bAttemptedLockpicking;

	// Parkour
	bool bIsClimbing;
	bool AttemptClimb();
	class UAnimMontage* ParkourMontage;

	// Behavior
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator, AActor* DamageCauser) override;
	virtual void Die() override;
	virtual void Jump() override;

	// Interaction
	UPROPERTY(Category = Interaction, VisibleAnywhere, BlueprintReadOnly)
	class USphereComponent* InteractablesDetector;
	float InteractablesDetectionRadius;
	TArray<AActor*> Interactables;
	void Interact();
	// If interaction button is being hold, attempts lockpicking, if it was just pressed, tries to open the door
	void InteractionHold();
	bool bCanInteract;
	void LerpProgress();
	void StopInteracting();
	float InteractionProgress;
	void EndInteractionSequence();
	FTimerHandle ProgressLerpHandle;
	class UAnimMontage* InteractionMontage;
	class UAnimMontage* LockpickingMontage;
	class AOpenable* CurrentOpenable;

	UFUNCTION()
		void DetectInteractable(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
			UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
			const FHitResult& SweepResult);

	UFUNCTION()
		void ForgetInteractable(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
			UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// UI
	UPROPERTY(EditAnywhere, category="UI")
	class UUIHandler* UIHandler;

	FVector2D ScreenMiddlePoint;

	// Inventory
	UPROPERTY(EditAnywhere, category = "Inventory")
		class UInventory* Inventory;

	void ToggleInventory();

	// Animation
	// functions fired when an anim montage has started or ended
	UFUNCTION()
	void OnMontageStarted(UAnimMontage* Montage);
	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	UFUNCTION()
	void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);

	// Vars
	UCapsuleComponent* CapsuleComp;



public:
	// Sets default values for this character's properties
	APlayerCharacter();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Movement
	bool bShouldTurnLeft;
	bool bShouldTurnRight;

	// UI
	void SetInputMode(bool bIsUI);
	FVector2D GetScreenMiddlePoint() const;

};
