// Copyright BanMing

#include "LSCharacterBase.h"

#include "Animation/AnimInstance.h"
#include "Animations/LSAnimInstance.h"
#include "Characters/LSCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Curves/CurveFloat.h"
#include "Curves/CurveVector.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

void ALSCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	OnBeginPlay();
}

void ALSCharacterBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	SetEssentialValues();

	// Check Movement Mode
	if (MovementState == ELSMovementState::Grounded)
	{
		UpdateCharacterMovement();
		UpdateGroundedRotation();
	}
	else if (MovementState == ELSMovementState::InAir)
	{
		UpdateInAirRotation();

		// Perform a mantle check if falling while movement input is pressed.
		if (bHasMovementInput)
		{
			// TODO MantleCheck()
		}
	}
	else if (MovementState == ELSMovementState::Ragdoll)
	{
		// TODO RagdollUpdate()
	}

	CacheValues();
}

#pragma region Input

void ALSCharacterBase::PlayerMovementInput(bool IsForwardAxis)
{
	if (MovementState == ELSMovementState::None || MovementState == ELSMovementState::Mantling || MovementState == ELSMovementState::Ragdoll)
	{
		return;
	}

	const float InputForward = GetInputAxisValue("MoveForward/Backwards");
	const float InputRight = GetInputAxisValue("MoveRight/Left");

	const float InputRightRange = UKismetMathLibrary::MapRangeClamped(FMath::Abs(InputRight), 0, 0.6f, 1.f, 1.2f);
	const float InputForwardRange = UKismetMathLibrary::MapRangeClamped(FMath::Abs(InputForward), 0, 0.6f, 1.f, 1.2f);
	const float MoveForward = FMath::Clamp(InputForward * InputRightRange, -1.f, 1.f);
	const float MoveRight = FMath::Clamp(InputRight * InputForwardRange, -1.f, 1.f);

	if (IsForwardAxis)
	{
		FVector Forward = UKismetMathLibrary::GetForwardVector(GetControlRotation());
		AddMovementInput(Forward, MoveForward);
	}
	else
	{
		FVector Right = UKismetMathLibrary::GetRightVector(GetControlRotation());
		AddMovementInput(Right, MoveRight);
	}
}

FVector ALSCharacterBase::GetPlayerMovementInput()
{
	FVector MovementInput = FVector::ZeroVector;

	FVector Forward = UKismetMathLibrary::GetForwardVector(GetControlRotation());
	FVector Right = UKismetMathLibrary::GetRightVector(GetControlRotation());

	const float InputForward = GetInputAxisValue("MoveForward/Backwards");
	const float InputRight = GetInputAxisValue("MoveRight/Left");

	MovementInput = Forward * InputForward + Right * InputRight;
	MovementInput.Normalize();

	return MovementInput;
}

#pragma endregion

#pragma region Essential Information

void ALSCharacterBase::GetMovementInfo(FMovementEssentialInfo& OutMovementInfo) const
{
	OutMovementInfo.Velocity = GetVelocity();
	OutMovementInfo.Acceleration = Acceleration;
	OutMovementInfo.MovementInput = GetCharacterMovement()->GetCurrentAcceleration();
	OutMovementInfo.bIsMoving = bIsMoving;
	OutMovementInfo.bHasMovementInput = bHasMovementInput;
	OutMovementInfo.Speed = Speed;
	OutMovementInfo.MovementInputAmount = MovementInputAmount;
	OutMovementInfo.AimRotation = GetControlRotation();
	OutMovementInfo.AimYawRate = AimYawRate;
}

void ALSCharacterBase::SetEssentialValues()
{
	// Set the amount of Acceleration.
	Acceleration = CalculateAcceleration();

	// Determine if the character is moving by getting it's speed.
	// The Speed equals the length of the horizontal (x y) velocity, so it does not take vertical movement into account.
	// If the character is moving, update the last velocity rotation.
	// This value is saved because it might be useful to know the last orientation of movement even after the character has stopped.
	Speed = GetVelocity().Size2D();
	bIsMoving = Speed > 1.f;
	if (bIsMoving)
	{
		LastVelocityRotation = GetVelocity().ToOrientationRotator();
	}

	// Determine if the character has movement input by getting its movement input amount.
	// The Movement Input Amount is equal to the current acceleration divided by the max acceleration
	// so that it has a range of 0 - 1, 1 being the maximum possible amount of input, and 0 beiung none.
	// If the character has movement input, update the Last Movement Input Rotation.
	const FVector CurrentAcceleration = GetCharacterMovement()->GetCurrentAcceleration();
	const float CurrentAccelerationLen = CurrentAcceleration.Length();
	MovementInputAmount = CurrentAccelerationLen / GetCharacterMovement()->GetMaxAcceleration();
	bHasMovementInput = MovementInputAmount > 0.f;
	if (bHasMovementInput)
	{
		LastMovementInputRotation = CurrentAcceleration.ToOrientationRotator();
	}

	// Set the Aim Yaw rate by comparing the current and previous Aim Yaw value, divided by Delta Seconds.
	// This represents the speed the camera is rotating left to right.
	AimYawRate = FMath::Abs((GetControlRotation().Yaw - PreviousAimYaw) / UGameplayStatics::GetWorldDeltaSeconds(this));
}

FVector ALSCharacterBase::CalculateAcceleration()
{
	return (GetVelocity() - PreviousVelocity) / UGameplayStatics::GetWorldDeltaSeconds(this);
}

void ALSCharacterBase::CacheValues()
{
	PreviousVelocity = GetVelocity();
	PreviousAimYaw = GetControlRotation().Yaw;
}

#pragma endregion

#pragma region Movement State Events

void ALSCharacterBase::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);
	OnCharacterMovementModeChanged(GetCharacterMovement()->MovementMode);
}

void ALSCharacterBase::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	OnStanceChanged(ELSStanceType::Crouching);
}

void ALSCharacterBase::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	OnStanceChanged(ELSStanceType::Standing);
}

void ALSCharacterBase::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	// TODO On Landed: Temporarily increase the braking friction on lands to make landings more accurate, or trigger a breakfall roll.
	// if (bBreakFall)
	//{
	//	BreakfallEvent();
	// }
	// else
	//{
	//	GetCharacterMovement()->BrakingFrictionFactor = bHasMovementInput ? 0.5f : 3.f;
	//	UKismetSystemLibrary::RetriggerableDelay(this, 0.5f);
	//	GetCharacterMovement()->BrakingFrictionFactor = 0.f;
	// }
}

void ALSCharacterBase::BreakfallEvent()
{
	// TODO Breakfall: Simply play a Root Motion Montage.
	if (IsValid(MainAnimInstance))
	{
		// MainAnimInstance->Montage_Play();
	}
}

void ALSCharacterBase::OnJumped_Implementation()
{
	Super::OnJumped_Implementation();
	// On Jumped : Set the new In Air Rotation to the velocity rotation if speed is greater than 100.
	InAirRotation = Speed > 100.f ? LastVelocityRotation : GetActorRotation();

	if (IsValid(MainAnimInstance))
	{
		// MainAnimInstance->Jumped();
	}
}

#pragma endregion

#pragma region State Changes

void ALSCharacterBase::GetMovementStates(FMovementStates& OutMovementStates) const
{
	OutMovementStates.PawnMovementMode = GetCharacterMovement()->MovementMode;
	OutMovementStates.MovementState = MovementState;
	OutMovementStates.PrevMovementState = PrevMovementState;
	OutMovementStates.MovementAction = MovementAction;
	OutMovementStates.RotationMode = RotationMode;
	OutMovementStates.ActualGait = Gait;
	OutMovementStates.ActualStance = Stance;
	OutMovementStates.ViewMode = ViewMode;
	OutMovementStates.OverlayState = OverlayState;
}

void ALSCharacterBase::OnBeginPlay()
{
	check(GetMesh());
	// Make sure the mesh and animbp update after the MovementComponent to ensure it gets the most recent values.

	GetMesh()->AddTickPrerequisiteActor(this);

	// Set Reference to the Main Anim Instance.
	MainAnimInstance = GetMesh()->GetAnimInstance();

	// Set the Movement Model
	SetMovementModel();

	// Update states to use the initial desired values.
	OnGaitChanged(DesiredGait);
	OnRotationModeChanged(DesiredRotationMode);
	OnViewModeChanged(ViewMode);
	OnOverlayStateChanged(OverlayState);
	if (DesiredStance == ELSStanceType::Standing)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}

	// Set default rotation values.
	TargetRotation = GetActorRotation();
	LastVelocityRotation = GetActorRotation();
	LastMovementInputRotation = GetActorRotation();
}

void ALSCharacterBase::OnCharacterMovementModeChanged(const EMovementMode& NewMovementMode)
{
	if (NewMovementMode == EMovementMode::MOVE_Walking || NewMovementMode == EMovementMode::MOVE_NavWalking)
	{
		OnMovementStateChanged(ELSMovementState::Grounded);
	}
	else if (NewMovementMode == EMovementMode::MOVE_Falling)
	{
		OnMovementStateChanged(ELSMovementState::InAir);
	}
}

void ALSCharacterBase::OnMovementStateChanged(const ELSMovementState& NewMovementState)
{
	if (MovementState == NewMovementState)
	{
		return;
	}

	MovementState = NewMovementState;

	// If the character enters the air, set the In Air Rotation and uncrouch if crouched.
	// If the character is currently rolling, enable the ragdoll.
	if (MovementState == ELSMovementState::InAir)
	{
		if (MovementAction == ELSMovementAction::None)
		{
			InAirRotation = GetActorRotation();
			if (Stance == ELSStanceType::Crouching)
			{
				UnCrouch();
			}
		}
		else if (MovementAction == ELSMovementAction::Rolling)
		{
			// TODO RagdollStart()
		}
	}
	else if (MovementState == ELSMovementState::InAir && PrevMovementState == ELSMovementState::Mantling)
	{
		// TODO StopMantling()
	}

	// Stop the Mantle Timeline if transitioning to the ragdoll state while mantling.
}

void ALSCharacterBase::OnMovementActionChanged(const ELSMovementAction& NewMovementAction)
{
	if (MovementAction == NewMovementAction)
	{
		return;
	}
	ELSMovementAction PrevMovementAction = MovementAction;
	MovementAction = NewMovementAction;
	// Make the character crouch if performing a roll.
	if (MovementAction == ELSMovementAction::Rolling)
	{
		Crouch();
	}

	// Upon ending a roll, reset the stance back to its desired value.
	if (PrevMovementAction == ELSMovementAction::Rolling)
	{
		if (DesiredStance == ELSStanceType::Crouching)
		{
			Crouch();
		}
		else
		{
			UnCrouch();
		}
	}
}

void ALSCharacterBase::OnStanceChanged(const ELSStanceType& NewStanceType)
{
	if (NewStanceType != Stance)
	{
		Stance = NewStanceType;
	}
}

void ALSCharacterBase::OnGaitChanged(const ELSGaitType& NewActualGait)
{
	if (Gait != NewActualGait)
	{
		Gait = NewActualGait;
	}
}

void ALSCharacterBase::OnRotationModeChanged(const ELSRotationMode& NewRotationMode)
{
	if (RotationMode == NewRotationMode)
	{
		return;
	}

	RotationMode = NewRotationMode;

	// If the new rotation mode is Velocity Direction and the character is in First Person, set the viewmode to Third Person.
	if (RotationMode == ELSRotationMode::VelocityDirection && ViewMode == ELSViewMode::FirstPerson)
	{
		OnViewModeChanged(ELSViewMode::ThirdPerson);
	}
}

void ALSCharacterBase::OnOverlayStateChanged(const ELSOverlayState& NewOverlayState)
{
	if (OverlayState != NewOverlayState)
	{
		OverlayState = NewOverlayState;
	}
}

void ALSCharacterBase::OnViewModeChanged(const ELSViewMode& NewViewMode)
{
	if (ViewMode == NewViewMode)
	{
		return;
	}

	ViewMode = NewViewMode;

	if (ViewMode == ELSViewMode::FirstPerson && RotationMode == ELSRotationMode::VelocityDirection)
	{
		// If Third Person, set the rotation mode back to the desired mode.
		OnRotationModeChanged(ELSRotationMode::LookingDirection);
	}
	else if (ViewMode == ELSViewMode::ThirdPerson && (RotationMode == ELSRotationMode::VelocityDirection || RotationMode == ELSRotationMode::LookingDirection))
	{
		// If First Person, set the rotation mode to looking direction if currently in the velocity direction mode.
		OnRotationModeChanged(DesiredRotationMode);
	}
}

#pragma endregion

#pragma region Movement System
void ALSCharacterBase::SetMovementModel()
{
	check(MovementModel.DataTable);
	MovementData = *MovementModel.DataTable->FindRow<FMovementSettings_State>(MovementModel.RowName, TEXT(""));
}

void ALSCharacterBase::UpdateCharacterMovement()
{
	// Set the Allowed Gait
	ELSGaitType AllowedGait = GetAllowedGait();

	// Determine the Actual Gait.If it is different from the current Gait, Set the new Gait Event.
	ELSGaitType ActualGait = GetActualGait(AllowedGait);
	if (Gait != ActualGait)
	{
		OnGaitChanged(ActualGait);
	}

	// Use the allowed gait to update the movement settings.
	UpdateDynamicMovementSettings(AllowedGait);
}

void ALSCharacterBase::UpdateDynamicMovementSettings(const ELSGaitType& AllowGait)
{
	// Set the Current Movement Settings.
	SetTargetMovementSettings();

	// Update the Character Max Walk Speed to the configured speeds based on the currently Allowed Gait.
	switch (AllowGait)
	{
		case ELSGaitType::Walking:
			GetCharacterMovement()->MaxWalkSpeed = CurMovementSettings.WalkSpeed;
			GetCharacterMovement()->MaxWalkSpeedCrouched = CurMovementSettings.WalkSpeed;
			break;
		case ELSGaitType::Running:
			GetCharacterMovement()->MaxWalkSpeed = CurMovementSettings.RunSpeed;
			GetCharacterMovement()->MaxWalkSpeedCrouched = CurMovementSettings.RunSpeed;
			break;
		case ELSGaitType::Sprinting:
			GetCharacterMovement()->MaxWalkSpeed = CurMovementSettings.SprintSpeed;
			GetCharacterMovement()->MaxWalkSpeedCrouched = CurMovementSettings.SprintSpeed;
			break;
	}

	// Update the Acceleration, Deceleration, and Ground Friction using the Movement Curve.
	// This allows for fine control over movement behavior at each speed (May not be suitable for replication).
	const FVector CurveValue = CurMovementSettings.MovementCurve->GetVectorValue(GetMappedSpeed());
	GetCharacterMovement()->MaxAcceleration = CurveValue.X;
	GetCharacterMovement()->BrakingDecelerationWalking = CurveValue.Y;
	GetCharacterMovement()->GroundFriction = CurveValue.Z;
}

void ALSCharacterBase::SetTargetMovementSettings()
{
	if (RotationMode == ELSRotationMode::VelocityDirection)
	{
		if (Stance == ELSStanceType::Standing)
		{
			CurMovementSettings = MovementData.VelocityDirection.Standing;
		}
		else if (Stance == ELSStanceType::Crouching)
		{
			CurMovementSettings = MovementData.VelocityDirection.Crouching;
		}
	}
	else if (RotationMode == ELSRotationMode::LookingDirection)
	{
		if (Stance == ELSStanceType::Standing)
		{
			CurMovementSettings = MovementData.LookingDirection.Standing;
		}
		else if (Stance == ELSStanceType::Crouching)
		{
			CurMovementSettings = MovementData.LookingDirection.Crouching;
		}
	}
	else if (RotationMode == ELSRotationMode::Aiming)
	{
		if (Stance == ELSStanceType::Standing)
		{
			CurMovementSettings = MovementData.Aiming.Standing;
		}
		else if (Stance == ELSStanceType::Crouching)
		{
			CurMovementSettings = MovementData.Aiming.Crouching;
		}
	}
}

ELSGaitType ALSCharacterBase::GetAllowedGait()
{
	ELSGaitType Res = ELSGaitType::Walking;
	if (Stance == ELSStanceType::Standing)
	{
		if (DesiredGait == ELSGaitType::Running)
		{
			Res = ELSGaitType::Running;
		}
		else if (DesiredGait == ELSGaitType::Sprinting && CanSprint())
		{
			Res = ELSGaitType::Sprinting;
		}
	}
	else if (Stance == ELSStanceType::Crouching && (DesiredGait == ELSGaitType::Running || DesiredGait == ELSGaitType::Sprinting))
	{
		Res = ELSGaitType::Running;
	}

	return Res;
}

ELSGaitType ALSCharacterBase::GetActualGait(const ELSGaitType& AllowGait)
{
	ELSGaitType Res = ELSGaitType::Walking;
	const float SpeedOffset = 10.f;
	if (Speed >= CurMovementSettings.RunSpeed + SpeedOffset)
	{
		if (AllowGait == ELSGaitType::Sprinting)
		{
			Res = ELSGaitType::Sprinting;
		}
		else
		{
			Res = ELSGaitType::Running;
		}
	}
	else if (Speed >= CurMovementSettings.WalkSpeed + SpeedOffset)
	{
		Res = ELSGaitType::Running;
	}

	return Res;
}

bool ALSCharacterBase::CanSprint() const
{
	bool bRes = false;
	if (!bHasMovementInput || RotationMode == ELSRotationMode::Aiming)
	{
		return bRes;
	}

	bool bIsOverInputAmount = MovementInputAmount > 0.9f;
	if (RotationMode == ELSRotationMode::VelocityDirection)
	{
		bRes = bIsOverInputAmount;
	}
	else if (RotationMode == ELSRotationMode::LookingDirection)
	{
		FRotator AccelertionRotator = GetCharacterMovement()->GetCurrentAcceleration().ToOrientationRotator();
		FRotator DeltaRotator = UKismetMathLibrary::NormalizedDeltaRotator(AccelertionRotator, GetControlRotation());
		bRes = bIsOverInputAmount && FMath::Abs(DeltaRotator.Yaw) < 50.f;
	}

	return bRes;
}

float ALSCharacterBase::GetMappedSpeed() const
{
	// Map the character's current speed to the configured movement speeds with a range of 0-3,
	// with 0 = stopped, 1 = the Walk Speed, 2 = the Run Speed, and 3 = the Sprint Speed.
	// This allows you to vary the movement speeds but still use the mapped range in calculations for consistent results.

	const float LocWalkSpeed = CurMovementSettings.WalkSpeed;
	const float LocRunSpeed = CurMovementSettings.RunSpeed;
	const float LocSprintSpeed = CurMovementSettings.SprintSpeed;

	const float WalkSpeed = UKismetMathLibrary::MapRangeClamped(Speed, 0.f, LocWalkSpeed, 0.f, 1.f);
	const float RunSpeed = UKismetMathLibrary::MapRangeClamped(Speed, LocWalkSpeed, LocRunSpeed, 1.f, 2.f);
	const float SprintSpeed = UKismetMathLibrary::MapRangeClamped(Speed, LocRunSpeed, LocSprintSpeed, 2.f, 3.);

	float MapSpeed = Speed > LocWalkSpeed ? RunSpeed : WalkSpeed;
	MapSpeed = Speed > LocRunSpeed ? SprintSpeed : MapSpeed;

	return MapSpeed;
}

UAnimMontage* ALSCharacterBase::GetRollAnimation()
{
	return nullptr;
}

#pragma endregion

#pragma region Rotation System

void ALSCharacterBase::UpdateGroundedRotation()
{
	// Rolling Rotation
	if (bHasMovementInput && MovementAction == ELSMovementAction::Rolling)
	{
		SmoothCharacterRotation(FRotator(0.f, LastMovementInputRotation.Yaw, 0.f), 0.f, 2.f);
	}

	if (CanUpdateMovingRotation())
	{
		switch (RotationMode)
		{
			case ELSRotationMode::VelocityDirection:
				SmoothCharacterRotation(FRotator(0.f, LastVelocityRotation.Yaw, 0.f), 800.f, CalculateGroundedRotationRate());
				break;
			case ELSRotationMode::LookingDirection:
			{
				float TargetYaw = LastVelocityRotation.Yaw;
				if (Gait == ELSGaitType::Walking || Gait == ELSGaitType::Running)
				{
					TargetYaw = GetControlRotation().Yaw + GetAnimCurveValue("YawOffset");
				}
				SmoothCharacterRotation(FRotator(0.f, TargetYaw, 0.f), 500.f, CalculateGroundedRotationRate());
			}
			break;
			case ELSRotationMode::Aiming:
				SmoothCharacterRotation(FRotator(0.f, GetControlRotation().Yaw, 0.f), 1000.f, 20.f);
				break;
		}
	}
	else
	{
		// Not Moving
		if (ViewMode == ELSViewMode::FirstPerson || RotationMode == ELSRotationMode::Aiming)
		{
			LimitRotation(-100, 100, 20);
		}

		// Apply the RotationAmount curve from Turn In Place Animations.
		// The Rotation Amount curve defines how much rotation should be applied each frame,
		// and is calculated for animations that are animated at 30fps.
		const float RotationAmount = GetAnimCurveValue("RotationAmount");
		if (FMath::Abs(RotationAmount) > 0.001f)
		{
			const float DeltaSec = UGameplayStatics::GetWorldDeltaSeconds(this) / 1 / 30.f;
			AddActorWorldRotation(FRotator(0.f, RotationAmount * DeltaSec, 0.f));
			TargetRotation = GetActorRotation();
		}
	}
}

void ALSCharacterBase::UpdateInAirRotation()
{
	// Velocity / Looking Direction Rotation
	if (RotationMode == ELSRotationMode::VelocityDirection || RotationMode == ELSRotationMode::LookingDirection)
	{
		SmoothCharacterRotation(FRotator(0.f, InAirRotation.Yaw, 0.f), 0.f, 5.f);
	}
	// Aiming Rotation
	else if (RotationMode == ELSRotationMode::Aiming)
	{
		SmoothCharacterRotation(FRotator(0.f, GetControlRotation().Yaw, 0.f), 0.f, 15.f);
		InAirRotation = GetActorRotation();
	}
}

void ALSCharacterBase::SmoothCharacterRotation(const FRotator& Target, float TargetInterpSpeed, float ActorInterpSpeed)
{
	const float DeltaTime = UGameplayStatics::GetWorldDeltaSeconds(this);
	TargetRotation = FMath::RInterpConstantTo(TargetRotation, Target, DeltaTime, TargetInterpSpeed);

	FRotator ActorRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, ActorInterpSpeed);
	SetActorRotation(ActorRotation);
}

void ALSCharacterBase::AddCharacterRotation(const FRotator& DeltaRotation)
{
	TargetRotation = UKismetMathLibrary::ComposeRotators(TargetRotation, DeltaRotation);
	AddActorWorldRotation(DeltaRotation);
}

void ALSCharacterBase::LimitRotation(float AimYawMin, float AimYawMax, float InterpSpeed)
{
	FRotator Delta = GetControlRotation() - GetActorRotation();
	Delta.Normalize();
	if (AimYawMin >= Delta.Yaw && Delta.Yaw <= AimYawMax)
	{
		return;
	}

	float TargetYaw = Delta.Yaw > 0.f ? GetControlRotation().Yaw + AimYawMin : GetControlRotation().Yaw + AimYawMax;
	SmoothCharacterRotation(FRotator(0.f, TargetYaw, 0.f), 0.f, InterpSpeed);
}

bool ALSCharacterBase::SetActorLocationAndRotationLoc(FVector NewLocation, FRotator NewRotation, bool bSweep, FHitResult* OutSweepHitResult, ETeleportType Teleport)
{
	TargetRotation = NewRotation;
	return SetActorLocationAndRotation(NewLocation, NewRotation, bSweep, OutSweepHitResult, Teleport);
}

float ALSCharacterBase::CalculateGroundedRotationRate() const
{
	const float CurveValue = CurMovementSettings.RotationRateCurve->GetFloatValue(GetMappedSpeed());
	const float AimYawValue = UKismetMathLibrary::MapRangeClamped(AimYawRate, 0.f, 300.f, 1.f, 3.f);
	return CurveValue * AimYawValue;
}

bool ALSCharacterBase::CanUpdateMovingRotation() const
{
	return (bIsMoving && bHasMovementInput || Speed > 150.f) && !HasAnyRootMotion();
}

#pragma endregion

#pragma region Utility
float ALSCharacterBase::GetAnimCurveValue(const FName& CurveName) const
{
	float Res = 0.f;
	if (IsValid(MainAnimInstance))
	{
		Res = MainAnimInstance->GetCurveValue(CurveName);
	}

	return Res;
}

FVector ALSCharacterBase::GetCapsuleBaseLocation(float ZOffset) const
{
	const UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	const float Height = CapsuleComp->GetScaledCapsuleHalfHeight() + ZOffset;

	return CapsuleComp->GetComponentLocation() - Height * CapsuleComp->GetUpVector();
}

FVector ALSCharacterBase::GetCapsuleLocationFromBase(const FVector& BaseLocation, float ZOffset) const
{
	const UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	const float Height = CapsuleComp->GetScaledCapsuleHalfHeight() + ZOffset;

	return BaseLocation + FVector(0.f, 0.f, Height);
}

#pragma endregion