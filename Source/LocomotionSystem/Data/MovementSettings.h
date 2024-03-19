// Copyright BanMing

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "MovementSettings.generated.h"

USTRUCT(BlueprintType)
struct FMovementSettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float WalkSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float RunSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float SprintSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class UCurveVector> MovementCurve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<class UCurveFloat> RotationRateCurve;
};

USTRUCT(BlueprintType)
struct FMovementSettings_Stance
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FMovementSettings Standing;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FMovementSettings Crouching;
};

USTRUCT(BlueprintType)
struct FMovementSettings_State : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FMovementSettings_Stance VelocityDirection;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FMovementSettings_Stance LookingDirection;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FMovementSettings_Stance Aiming;
};
