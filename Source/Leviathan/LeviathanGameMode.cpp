// Copyright Epic Games, Inc. All Rights Reserved.

#include "LeviathanGameMode.h"
#include "LeviathanCharacter.h"
#include "UObject/ConstructorHelpers.h"

ALeviathanGameMode::ALeviathanGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
