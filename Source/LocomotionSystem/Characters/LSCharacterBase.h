// Copyright BanMing

#pragma once

#include "CoreMinimal.h"
#include "Data/MovementSettings.h"
#include "Engine/DataTable.h"
#include "GameFramework/Character.h"

#include "LSCharacterBase.generated.h"

UENUM(BlueprintType)
enum class ELSGaitType
{
	Walking,
	Running,
	Sprinting
};

UENUM(BlueprintType)
enum class ELSStanceType
{
	Standing,
	Crouching,
};

UENUM(BlueprintType)
enum class ELSRotationMode
{
	VelocityDirection,
	LookingDirection,
	Aiming
};

UENUM(BlueprintType)
enum class ELSViewMode
{
	ThirdPerson,
	FirstPerson,
};

UENUM(BlueprintType)
enum class ELSMovementState
{
	None,
	Grounded,
	InAir,
	Mantling,
	Ragdoll
};

UENUM(BlueprintType)
enum class ELSOverlayState
{
	Default,
	Masculine,
	Feminine,
	Injured,
	HandsTied,
	Rifle,
	Pistol1H,
	Pistol2H,
	Bow,
	Torch,
	Binoculars,
	Box,
	Barrel,
};

UCLASS(config = Game)
class ALSCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

#pragma region References
protected:
	UPROPERTY()
	TObjectPtr<class UAnimInstance> MainAnimInstance;

#pragma endregion

#pragma region Input
protected:
	void PlayerMovementInput(bool IsForwardAxis);
	FVector GetPlayerMovementInput();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Locomotion|Input")
	ELSRotationMode DesiredRotationMode = ELSRotationMode::LookingDirection;

	UPROPERTY(EditDefaultsOnly, Category = "Locomotion|Input")
	ELSGaitType DesiredGait = ELSGaitType::Running;

	UPROPERTY(EditDefaultsOnly, Category = "Locomotion|Input")
	ELSStanceType DesiredStance = ELSStanceType::Standing;

	UPROPERTY(EditDefaultsOnly, Category = "Locomotion|Input")
	float LookUpDownRate = 1.25f;

	UPROPERTY(EditDefaultsOnly, Category = "Locomotion|Input")
	float LookLeftRightRate = 1.25f;

	UPROPERTY(EditDefaultsOnly, Category = "Locomotion|Input")
	int TimesPressedStance = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Locomotion|Input")
	bool BreakFall = false;

	UPROPERTY(EditDefaultsOnly, Category = "Locomotion|Input")
	bool SprintHeld = false;
#pragma endregion

#pragma region Essential Information
protected:
	FRotator LastVelocityRotation = FRotator::ZeroRotator;
	FRotator LastMovementInputRotation = FRotator::ZeroRotator;

#pragma endregion

#pragma region State Changes
protected:
	void OnGaitChanged(const ELSGaitType& NewActualGait);
	void OnRotationModeChanged(const ELSRotationMode& NewRotationMode);
	void OnOverlayStateChanged(const ELSOverlayState& NewOverlayState);
	void OnViewModeChanged(const ELSViewMode& NewViewMode);

protected:
	UPROPERTY(EditDefaultsOnly)
	ELSGaitType Gait = ELSGaitType::Walking;

	UPROPERTY(EditDefaultsOnly)
	ELSRotationMode RotationMode = ELSRotationMode::LookingDirection;

	UPROPERTY(EditDefaultsOnly)
	ELSViewMode ViewMode = ELSViewMode::ThirdPerson;

	UPROPERTY(EditDefaultsOnly)
	ELSOverlayState OverlayState = ELSOverlayState::Default;

	UPROPERTY(EditDefaultsOnly)
	ELSMovementState MovementState = ELSMovementState::None;

	UPROPERTY(EditDefaultsOnly)
	ELSMovementState PrevMovementState = ELSMovementState::None;
#pragma endregion

#pragma region Rotation System

protected:
	FRotator TargetRotation = FRotator::ZeroRotator;

#pragma endregion

#pragma region Movement Data
protected:
	UPROPERTY()
	FMovementSettings_Stance MovementData;

	UPROPERTY(EditDefaultsOnly)
	FDataTableRowHandle MovementModel;
#pragma endregion
};
