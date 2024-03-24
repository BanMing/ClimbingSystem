// Copyright BanMing

#include "Animations/LSAnimInstance.h"

#include "Characters/LSCharacterBase.h"
#include "Curves/CurveFloat.h"
#include "GameFramework/CharacterMovementComponent.h"

void ULSAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	if (ALSCharacterBase* LSCharacter = Cast<ALSCharacterBase>(TryGetPawnOwner()))
	{
		Character = LSCharacter;
		CharacterMovementComp = Character->GetCharacterMovement();
	}
}

void ULSAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	DeltaTimeX = DeltaSeconds;

	if (DeltaTimeX == 0.f || IsValid(Character))
	{
		return;
	}

	Character->GetMovementInfo(MovementInfo);
	Character->GetMovementStates(MovementStates);
}

#pragma region Movement
void ULSAnimInstance::UpdateMovementValues()
{
	// Interp and set the Velocity Blend.
	FVelocityBlend Target = CalculateVelocityBlend();
	VelocityBlend.F = FMath::FInterpTo(VelocityBlend.F, Target.F, DeltaTimeX, VelocityBlendInterpSpeed);
	VelocityBlend.B = FMath::FInterpTo(VelocityBlend.B, Target.B, DeltaTimeX, VelocityBlendInterpSpeed);
	VelocityBlend.L = FMath::FInterpTo(VelocityBlend.L, Target.L, DeltaTimeX, VelocityBlendInterpSpeed);
	VelocityBlend.R = FMath::FInterpTo(VelocityBlend.R, Target.R, DeltaTimeX, VelocityBlendInterpSpeed);
}

FVelocityBlend ULSAnimInstance::CalculateVelocityBlend()
{
	const FVector LocRelativeVelocityDir = Character->GetActorRotation().UnrotateVector(MovementInfo.Velocity.GetSafeNormal());
	float Sum = FMath::Abs(LocRelativeVelocityDir.X) + FMath::Abs(LocRelativeVelocityDir.Y) + FMath::Abs(LocRelativeVelocityDir.Z);
	const FVector RelativeDirection = LocRelativeVelocityDir / Sum;

	FVelocityBlend Res;
	Res.F = FMath::Clamp(RelativeDirection.X, 0.f, 1.f);
	Res.B = FMath::Abs(FMath::Clamp(RelativeDirection.X, -1.f, 0.f));
	Res.L = FMath::Abs(FMath::Clamp(RelativeDirection.Y, -1.f, 0.f));
	Res.R = FMath::Clamp(RelativeDirection.Y, 0.f, 1.f);

	return Res;
}

float ULSAnimInstance::CalculateDiagonalScaleAmount()
{
	// This value is used to scale the Foot IK Root bone to make the Foot IK bones cover more distance on the diagonal blends.
	// Without scaling, the feet would not move far enough on the diagonal direction
	// due to the linear translational blending of the IK bones.The curve is used to easily map the value.
	return DiagonalScaleAmountCurve->GetFloatValue(FMath::Abs(VelocityBlend.F + VelocityBlend.B));
}

FVector ULSAnimInstance::CalculateRelativeAccelerationAmount()
{
	// This value represents the current amount of acceleration / deceleration relative to the actor rotation.
	// It is normalized to a range of - 1 to 1 so that - 1 equals the Max Braking Deceleration,
	// and 1 equals the Max Acceleration of the Character Movement Component.

	check(CharacterMovementComp);

	const bool bIsSameDir = MovementInfo.Acceleration.Dot(MovementInfo.Velocity) > 0.f;
	FVector Res = FVector::ZeroVector;
	if (bIsSameDir)
	{
		const float MaxAcceleration = CharacterMovementComp->GetMaxAcceleration();
		const FVector AccelerationNormal = MovementInfo.Acceleration.GetClampedToMaxSize(MaxAcceleration) / MaxAcceleration;
		Res = Character->GetActorRotation().UnrotateVector(AccelerationNormal);
	}
	else
	{
		const float MaxBrakingDeceleration = CharacterMovementComp->GetMaxBrakingDeceleration();
		const FVector AccelerationNormal = MovementInfo.Acceleration.GetClampedToMaxSize(MaxBrakingDeceleration) / MaxBrakingDeceleration;
		Res = Character->GetActorRotation().UnrotateVector(AccelerationNormal);
	}

	return Res;
}

float ULSAnimInstance::CalculateWalkRunBlend()
{
	// This value is used within the Blendspaces to blend between walking and running.
	float Res = 0.f;
	if (MovementStates.ActualGait == ELSGaitType::Running || MovementStates.ActualGait == ELSGaitType::Sprinting)
	{
		Res = 1.0f;
	}
	return 0.0f;
}

float ULSAnimInstance::CalculateStrideBlend()
{
	// This value is used within the blendspaces to scale the stride (distance feet travel)
	// so that the character can walk or run at different movement speeds.
	// It also allows the walk or run gait animations to blend independently
	// while still matching the animation speed to the movement speed,
	// preventing the character from needing to play a half walk+half run blend.
	// The curves are used to map the stride amount to the speed for maximum control.

	const float WalkValue = StrideBlend_N_Walk->GetFloatValue(MovementInfo.Speed);
	const float RunValue = StrideBlend_N_Run->GetFloatValue(MovementInfo.Speed);
	const float StanceValue = FMath::Lerp(WalkValue, RunValue, GetAnimCurveClamped("Weight_Gait"));

	const float CrouchValue = StrideBlend_C_Walk->GetFloatValue(MovementInfo.Speed);

	return FMath::Lerp(StanceValue, CrouchValue, GetAnimCurveClamped("BasePose_CLF"));
}

float ULSAnimInstance::CalculateStandingPlayRate()
{
	// The lerps are determined by the "Weight_Gait" anim curve that exists on every locomotion cycle
	// so that the play rate is always in sync with the currently blended animation.
	// The value is also divided by the Stride Blend and the mesh scale so that the play rate increases as the stride or scale gets smaller.
	float GaitCurveValue = FMath::Lerp(MovementInfo.Speed / AnimatedWalkSpeed, MovementInfo.Speed / AnimatedRunSpeed, GetAnimCurveClamped("Weight_Gait"));

	GaitCurveValue = FMath::Lerp(GaitCurveValue, MovementInfo.Speed / AnimatedSprintSpeed, GetAnimCurveClamped("Weight_Gait", -2.f));

	const float PlayRate = GaitCurveValue / StrideBlend / GetOwningComponent()->GetComponentScale().Z;

	return FMath::Clamp(PlayRate, 0.f, 3.f);
}

float ULSAnimInstance::CalculateCrouchingPlayRate()
{
	// This value needs to be separate from the standing play rate to improve the blend from crouch to stand while in motion.
	const float PlayRate = MovementInfo.Speed / AnimatedCrouchSpeed / StrideBlend / GetOwningComponent()->GetComponentScale().Z;
	return FMath::Clamp(PlayRate, 0.f, 2.f);
}

#pragma endregion