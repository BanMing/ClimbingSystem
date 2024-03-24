// Copyright BanMing

#pragma once

#include "Animation/AnimInstance.h"
#include "Characters/LSCharacterBase.h"
#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"

#include "LSAnimInstance.generated.h"

USTRUCT(BlueprintType)
struct FMovementEssentialInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion|Essential Info")
	FVector Velocity = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion|Essential Info")
	FVector Acceleration = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion|Essential Info")
	FVector MovementInput = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion|Essential Info")
	bool bIsMoving = false;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion|Essential Info")
	bool bHasMovementInput = false;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion|Essential Info")
	float Speed = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion|Essential Info")
	float MovementInputAmount = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion|Essential Info")
	FRotator AimRotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion|Essential Info")
	float AimYawRate = 0.f;
};

USTRUCT(BlueprintType)
struct FMovementStates
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion|State")
	TEnumAsByte<enum EMovementMode> PawnMovementMode;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion|State")
	ELSMovementState MovementState = ELSMovementState::None;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion|State")
	ELSMovementState PrevMovementState = ELSMovementState::None;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion|State")
	ELSMovementAction MovementAction = ELSMovementAction::None;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion|State")
	ELSRotationMode RotationMode = ELSRotationMode::LookingDirection;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion|State")
	ELSGaitType ActualGait = ELSGaitType::Walking;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion|State")
	ELSStanceType ActualStance = ELSStanceType::Standing;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion|State")
	ELSViewMode ViewMode = ELSViewMode::ThirdPerson;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion|State")
	ELSOverlayState OverlayState = ELSOverlayState::Default;
};

/**
 * This value represents the velocity amount of the actor in each direction
 * (normalized so that diagonals equal .5 for each direction),
 * and is used in a BlendMulti node to produce better directional blending than a standard blendspace.
 */
USTRUCT(BlueprintType)
struct FVelocityBlend
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly)
	float F = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float B = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float L = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float R = 0.f;
};

UENUM(BlueprintType)
enum class ELSGroundedEntryState
{
	None,
	Roll
};

UENUM(BlueprintType)
enum class ELSHipsDirection
{
	F,
	B,
	RF,
	RB,
	LF,
	LB,
};

UENUM(BlueprintType)
enum class ELSMovementDirection
{
	Forward,
	Right,
	Left,
	Backward
};

class UCurveFloat;
/**
 *
 */
UCLASS()
class LOCOMOTIONSYSTEM_API ULSAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

#pragma region Helpers

	inline float GetAnimCurveClamped(const FName& Name, float Bias = -1.f, float ClampMin = 0.f, float ClampMax = 1.0f)
	{
		return FMath::Clamp(GetCurveValue(Name) + Bias, ClampMin, ClampMax);
	}

#pragma endregion

#pragma region Aiming
protected:
	FRotator SmoothedAimingRotation;
	FRotator SpineRotation;
	FVector2D AimingAngle;
	FVector2D SmoothedAimingAngle;
	float AimSweepTime;
	float InputYawTime;
	float ForwardYawTime;
	float LeftTawTime;
	float RightYawTime;
#pragma endregion

#pragma region Grounded

#pragma endregion

#pragma region Movement
protected:
	void UpdateMovementValues();

	FVelocityBlend CalculateVelocityBlend();

	// Calculate the Diagonal Scale Amount.
	float CalculateDiagonalScaleAmount();

	// Calculate the Relative Acceleration Amount.
	FVector CalculateRelativeAccelerationAmount();

	// Calculate the Walk Run Blend.
	float CalculateWalkRunBlend();

	// Calculate the Stride Blend.
	float CalculateStrideBlend();

	// Calculate the Play Rate by dividing the Character's speed by the Animated Speed for each gait.
	float CalculateStandingPlayRate();

	// Calculate the Crouching Play Rate by dividing the Character's speed by the Animated Speed.
	float CalculateCrouchingPlayRate();

protected:
	FVelocityBlend VelocityBlend;
	float StrideBlend = 0.f;
	float StandingPlayRate = 1.f;
	float WalkRunBlend = 0.f;
	float CrouchingPlayRate = 1.f;
#pragma endregion

protected:
	UPROPERTY()
	TObjectPtr<class ALSCharacterBase> Character;

	UPROPERTY()
	TObjectPtr<class UCharacterMovementComponent> CharacterMovementComp;

	float DeltaTimeX = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion|Essential Info")
	FMovementEssentialInfo MovementInfo;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion|State")
	FMovementStates MovementStates;

#pragma region Config
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	float AnimatedWalkSpeed = 150.f;

	UPROPERTY(EditDefaultsOnly, Category = "Config")
	float AnimatedRunSpeed = 350.f;

	UPROPERTY(EditDefaultsOnly, Category = "Config")
	float AnimatedSprintSpeed = 600.f;

	UPROPERTY(EditDefaultsOnly, Category = "Config")
	float AnimatedCrouchSpeed = 150.f;

	UPROPERTY(EditDefaultsOnly, Category = "Config")
	float VelocityBlendInterpSpeed = 12.f;
#pragma endregion

#pragma region Blend Curves
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Blend Curves")
	TObjectPtr<UCurveFloat> DiagonalScaleAmountCurve;

	UPROPERTY(EditDefaultsOnly, Category = "Blend Curves")
	TObjectPtr<UCurveFloat> StrideBlend_N_Walk;

	UPROPERTY(EditDefaultsOnly, Category = "Blend Curves")
	TObjectPtr<UCurveFloat> StrideBlend_N_Run;

	UPROPERTY(EditDefaultsOnly, Category = "Blend Curves")
	TObjectPtr<UCurveFloat> StrideBlend_C_Walk;

#pragma endregion
};
