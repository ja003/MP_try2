// Copyright Epic Games, Inc. All Rights Reserved.

#include "MP_try2GameMode.h"
#include "MP_try2Character.h"
#include "UObject/ConstructorHelpers.h"

AMP_try2GameMode::AMP_try2GameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
