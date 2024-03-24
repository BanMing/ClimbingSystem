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
enum class ELSMovementAction
{
	None,
	LowMantle,
	HighMantle,
	Rolling,
	GettingUp
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

class UAnimMontage;

UCLASS(config = Game)
class ALSCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
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
	bool bBreakFall = false;

	UPROPERTY(EditDefaultsOnly, Category = "Locomotion|Input")
	bool bSprintHeld = false;
#pragma endregion

#pragma region Essential Information
public:
	 void GetMovementInfo(struct FMovementEssentialInfo& OutMovementInfo) const;

protected:
	// These values represent how the capsule is moving as well as how it wants to move,
	//  and therefore are essential for any data driven animation system.
	//  They are also used throughout the system for various functions, so I found it is easiest to manage them all in one place.
	void SetEssentialValues();

	// Calculate the Acceleration by comparing the current and previous velocity.
	// The Current Acceleration returned by the movement component equals the input acceleration,
	// and does not represent the actual physical acceleration of the character.
	FVector CalculateAcceleration();

	// Cache certain values to be used in calculations on the next frame
	void CacheValues();

protected:
	FVector Acceleration;
	bool bIsMoving = false;
	bool bHasMovementInput = false;
	FRotator LastVelocityRotation = FRotator::ZeroRotator;
	FRotator LastMovementInputRotation = FRotator::ZeroRotator;
	float Speed;
	float MovementInputAmount;
	float AimYawRate;

	FVector PreviousVelocity = FVector::ZeroVector;
	float PreviousAimYaw = 0.f;
#pragma endregion

#pragma region State Events
public:
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode = 0) override;

	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	virtual void Landed(const FHitResult& Hit) override;

	void BreakfallEvent();
	virtual void OnJumped_Implementation() override;
#pragma endregion

#pragma region State Changes
public:
	void GetMovementStates(struct FMovementStates& OutMovementStates) const;

protected:
	void OnBeginPlay();

	// Use the Character Movement Mode changes to set the Movement States to the right values.
	// This allows you to have a custom set of movement states
	// but still use the functionality of the default character movement component.
	void OnCharacterMovementModeChanged(const EMovementMode& NewMovementMode);
	void OnMovementStateChanged(const ELSMovementState& NewMovementState);
	void OnMovementActionChanged(const ELSMovementAction& NewMovementAction);
	void OnStanceChanged(const ELSStanceType& NewStanceType);
	void OnGaitChanged(const ELSGaitType& NewActualGait);
	void OnRotationModeChanged(const ELSRotationMode& NewRotationMode);
	void OnOverlayStateChanged(const ELSOverlayState& NewOverlayState);
	void OnViewModeChanged(const ELSViewMode& NewViewMode);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Locomotion|State")
	ELSMovementState MovementState = ELSMovementState::None;

	UPROPERTY(EditDefaultsOnly, Category = "Locomotion|State")
	ELSMovementState PrevMovementState = ELSMovementState::None;

	UPROPERTY(EditDefaultsOnly, Category = "Locomotion|State")
	ELSMovementAction MovementAction = ELSMovementAction::None;

	UPROPERTY(EditDefaultsOnly, Category = "Locomotion|State")
	ELSRotationMode RotationMode = ELSRotationMode::LookingDirection;

	UPROPERTY(EditDefaultsOnly, Category = "Locomotion|State")
	ELSGaitType Gait = ELSGaitType::Walking;

	UPROPERTY(EditDefaultsOnly, Category = "Locomotion|State")
	ELSStanceType Stance = ELSStanceType::Standing;

	UPROPERTY(EditDefaultsOnly, Category = "Locomotion|State")
	ELSViewMode ViewMode = ELSViewMode::ThirdPerson;

	UPROPERTY(EditDefaultsOnly, Category = "Locomotion|State")
	ELSOverlayState OverlayState = ELSOverlayState::Default;

#pragma endregion

#pragma region Movement System
protected:
	// Get movement data from the Movement Model Data table and set the Movement Data Struct.
	// This allows you to easily switch out movement behaviors.
	void SetMovementModel();
	void UpdateCharacterMovement();
	void UpdateDynamicMovementSettings(const ELSGaitType& AllowGait);
	void SetTargetMovementSettings();

	ELSGaitType GetAllowedGait();
	ELSGaitType GetActualGait(const ELSGaitType& AllowGait);

	// Determine if the character is currently able to sprint based on the Rotation mode and current acceleration(input) rotation.
	// If the character is in the Looking Rotation mode, only allow sprinting if there is full movement input and
	// it is faced forward relative to the camera + or -50 degrees.
	bool CanSprint() const;

	float GetMappedSpeed() const;
	virtual UAnimMontage* GetRollAnimation();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion|Movement")
	FMovementSettings_State MovementData;

	UPROPERTY(EditDefaultsOnly, Category = "Locomotion|Movement")
	FDataTableRowHandle MovementModel;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion|Movement")
	FMovementSettings CurMovementSettings;
#pragma endregion

#pragma region Rotation System
protected:
	void UpdateGroundedRotation();

	void UpdateInAirRotation();

	// Interpolate the Target Rotation for extra smooth rotation behavior
	void SmoothCharacterRotation(const FRotator& Target, float TargetInterpSpeed, float ActorInterpSpeed);

	void AddCharacterRotation(const FRotator& DeltaRotation);

	// Prevent the character from rotating past a certain angle.
	void LimitRotation(float AimYawMin, float AimYawMax, float InterpSpeed);

	// Update the Actors Location and Rotation as well as the Target Rotation variable to keep everything in sync.
	bool SetActorLocationAndRotationLoc(FVector NewLocation, FRotator NewRotation, bool bSweep = false, FHitResult* OutSweepHitResult = nullptr, ETeleportType Teleport = ETeleportType::None);
	float CalculateGroundedRotationRate() const;
	bool CanUpdateMovingRotation() const;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion|Rotation")
	FRotator TargetRotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion|Rotation")
	FRotator InAirRotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion|Rotation")
	float YawOffset = 0.f;
#pragma endregion

#pragma region Utility
	float GetAnimCurveValue(const FName& CurveName) const;
	FVector GetCapsuleBaseLocation(float ZOffset) const;
	FVector GetCapsuleLocationFromBase(const FVector& BaseLocation, float ZOffset) const;
#pragma endregion
};
