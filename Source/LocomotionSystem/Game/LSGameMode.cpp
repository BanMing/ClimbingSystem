// Copyright Epic Games, Inc. All Rights Reserved.

#include "LSGameMode.h"

#include "Characters/LSCharacter.h"
#include "UObject/ConstructorHelpers.h"

ALSGameMode::ALSGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
