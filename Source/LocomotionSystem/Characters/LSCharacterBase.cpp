// Copyright BanMing

#include "LSCharacterBase.h"

#include "Animation/AnimInstance.h"
#include "Characters/LSCharacter.h"
#include "Kismet/KismetMathLibrary.h"

void ALSCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	check(GetMesh());
	// Make sure the mesh and animbp update after the MovementComponent to ensure it gets the most recent values.

	GetMesh()->AddTickPrerequisiteActor(this);

	// Set Reference to the Main Anim Instance.
	MainAnimInstance = GetMesh()->GetAnimInstance();

	// Set the Movement Model
	// Get movement data from the Movement Model Data table and set the Movement Data Struct.This allows you to easily switch out movement behaviors.
	check(MovementModel.DataTable);
	MovementData = *MovementModel.DataTable->FindRow<FMovementSettings_Stance>(MovementModel.RowName, TEXT(""));

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

#pragma region Input

void ALSCharacterBase::PlayerMovementInput(bool IsForwardAxis)
{
	if (MovementState == ELSMovementState::None || MovementState == ELSMovementState::Mantling || MovementState == ELSMovementState::Ragdoll)
	{
		return;
	}

	FVector Forward = UKismetMathLibrary::GetForwardVector(GetControlRotation());
	FVector Right = UKismetMathLibrary::GetRightVector(GetControlRotation());
	//getmove
	if (IsForwardAxis)
	{
	}
	else
	{
	}
	//FixDiagonalGamepadValues
	// AddMovementInput()
}

FVector ALSCharacterBase::GetPlayerMovementInput()
{
	FVector Forward = UKismetMathLibrary::GetForwardVector(GetControlRotation());
	FVector Right = UKismetMathLibrary::GetRightVector(GetControlRotation());
	return FVector();
}

#pragma endregion

#pragma region State Changes

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