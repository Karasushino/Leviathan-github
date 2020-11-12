// Fill out your copyright notice in the Description page of Project Settings.

#include "CLeviathanCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Templates/Casts.h"

//////////////////////////////////////////////////////////////////////////
// ALeviathanCharacter

ACLeviathanCharacter::ACLeviathanCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	IdleCameraVector = FVector(10.0f,95.0f,20.0f);
	AimCameraVector = FVector(10.0f,80.0f,18.0f);
	// set our turn rates for input
	BaseTurnRate = NormalTurnRate;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	//Create a child component for the axe.
	LeviathanAxeChildActorComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("LeviathanAxe"));
	LeviathanAxeChildActorComponent->SetupAttachment(this->GetMesh(), "RightHandWeaponBoneSocket");

	//Gets a reference of the Leviathan Axe from the child actor component (or atleast should)

	
	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

void ACLeviathanCharacter::BeginPlay()
{
	//Calls Parents Begin Play
	Super::BeginPlay();
	//Setup begin play here
	LeviathanAxe = Cast<ALeviathanAxe>(LeviathanAxeChildActorComponent);
	
	//I need to get a reference of the Blueprint here
}




//////////////////////////////////////////////////////////////////////////
// Input

void ACLeviathanCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);

	//Axe controls
	//PlayerInputComponent->BindAxis("Aim", this,&ALeviathanCharacter::Aim);

	
	PlayerInputComponent->BindAxis("MoveForward", this, &ACLeviathanCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ACLeviathanCharacter::MoveRight);
	
	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ACLeviathanCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ACLeviathanCharacter::LookUpAtRate);

}

void ACLeviathanCharacter::Aim()
{
	//If the value is 1 it means player is aiming, else its released.
	if(bAiming)
	{
		bAiming = true;
		//Decrease Sens
		BaseTurnRate = AimingCameraTurnRate;
		//Need to add hud here
		//Need to set ON ranged attack mode from animation blueprint
		//Use controller rotation so player looks where he is aiming
		bUseControllerRotationYaw = true;

		GetCharacterMovement()->MaxWalkSpeed = AimWalkSpeed;
	}
	//If not Aiming/Aim Released
	else
	{
		bAiming = false;
		//Restore
		BaseTurnRate = NormalTurnRate;
		//Need to hide HUD here
		//Need to set OFF ranged attack mode from animation blueprint
		bUseControllerRotationYaw = false;
		GetCharacterMovement()->MaxWalkSpeed = IdleWalkSpeed;

	}
}

void ACLeviathanCharacter::LerpCameraPosition(float LerpCurve)
{
	CameraBoom->TargetArmLength = FMath::Lerp(IdleSpringArmLength,AimSpringArmLength,LerpCurve);
	LerpedSocketOffset = FMath::Lerp(IdleCameraVector,AimCameraVector,LerpCurve);
	CameraBoom->SocketOffset = LerpedSocketOffset;
	
}

void ACLeviathanCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ACLeviathanCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ACLeviathanCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ACLeviathanCharacter::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}
