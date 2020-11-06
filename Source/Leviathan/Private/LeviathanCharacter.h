// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "LeviathanAxe.h"
#include "LeviathanCharacter.generated.h"

UCLASS(config=Game)
class ALeviathanCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	
	
	
public:
	ALeviathanCharacter();
	//Get a pointer to the Axe in the world.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Axe)
	class ALeviathanAxe* LeviathanAxe;
	//Possibly will need a reference to the HUD later on here
	//Imaginary hud ref

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
	
	/** Axe Child Object */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Axe)
	class UChildActorComponent* LeviathanAxeChildActorComponent;
	
	/** Turn rate variable that will be passed to the camera for processing */
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

#pragma region Character Camera Variables
	//Different Zooms when aimed and idle
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float IdleSpringArmLength;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float AimSpringArmLength;
	//Different Camera Vector moves position of camera.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	FVector IdleCameraVector;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	FVector AimCameraVector;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	FVector LerpedSocketOffset;
	/**The turn rate when aiming*/
	UPROPERTY(VisibleAnywhere,Category=Camera)
	float AimingCameraTurnRate = 30.f;
	/**The turn rate when not aiming*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float NormalTurnRate = 50.f;
	
	
#pragma endregion

#pragma region Movement Variables
	//Change movement speed when aiming down.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Movement)
	float AimWalkSpeed = 250.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Movement)
	float IdleWalkSpeed = 500.f;

#pragma endregion 
	
protected:
	
	
	//New begin play to set up stuff
	virtual void BeginPlay() override;

	
#pragma region Movement Functions
	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
#pragma endregion
#pragma region Aiming Functions

	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Called for forwards/backward input */
	UFUNCTION(BlueprintCallable, Category = AxeAim)
	void Aim();
	
	UFUNCTION(BlueprintCallable, Category = AxeAim)
	void LerpCameraPosition(float LerpCurve);
	
#pragma endregion 

#pragma region Throw Functions
	
#pragma endregion 
	
protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:
	//Boolean to keep track to see if the player aimed and can shoot or not.
	UPROPERTY(BlueprintReadWrite,Category = "AxeAiming")
	bool bAiming = false;

	//Boolean to keep track to see if the player has thrown the axe already or not
	UPROPERTY(BlueprintReadWrite,Category = "AxeThrow")
	bool bAxeThrown = false;
	
#pragma region Camera Components
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	/** Throw axe function */
	UFUNCTION(BlueprintCallable, Category = AxeThrow)
    void ThrowAxe();
	
#pragma endregion 
};

