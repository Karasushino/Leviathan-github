﻿// Fill out your copyright notice in the Description page of Project Settings.

#include "LeviathanAxe.h"

#include "DrawDebugHelpers.h"
#include "LeviathanCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
ALeviathanAxe::ALeviathanAxe()

{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Get reference to the player
	
#pragma region Component Initalizer Variables
	
	//Create a root component.
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = Root;
	//Create Center Pivot point for spinning the axe in the air.
	CenterPoint = CreateDefaultSubobject<USceneComponent>(TEXT("CenterPoint"));
	CenterPoint->SetupAttachment(RootComponent);

	//Pivot point on the blade of the axe to adjust to surfaces when attached to them.
	LodgePoint = CreateDefaultSubobject<USceneComponent>(TEXT("LodgePoint"));
	LodgePoint->SetupAttachment(CenterPoint);

	//The Mesh for the Axe.
	AxeMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("AxeMesh"));
	AxeMesh->SetupAttachment(LodgePoint);
	//Projectile component for projectile calculations.
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));

	//Particle System.
	ThrowParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ThrowParticles"));
	ThrowParticles->SetupAttachment(AxeMesh);
	SwingParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("SwingParticles"));
	SwingParticles->SetupAttachment(AxeMesh);
	AxeCatchParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("AxeCatchParticles"));
	AxeCatchParticles->SetupAttachment(AxeMesh);
	

#pragma endregion
}




// Called when the game starts or when spawned
void ALeviathanAxe::BeginPlay()
{
	Super::BeginPlay();
	Player = Cast<ALeviathanCharacter>(GetWorld()->GetFirstPlayerController()->GetCharacter());
}



void ALeviathanAxe::Throw()
{
	
	//Only execute if player was aiming and the axe was not thrown already
	if(!Player->bAxeThrown)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Throw was called"));
		//Detach the Axe from the player
		this->DetachFromActor(FDetachmentTransformRules(EDetachmentRule::KeepWorld,true));
		//
		//Get all Data for maths
		ThrowCameraRotator = Player->FollowCamera->GetComponentRotation();
		ThrowDirection = Player->FollowCamera->GetForwardVector();
		ThrowCameraLocation = Player->FollowCamera->GetComponentLocation();
		
		
		MoveAxeToStartPosition();
		ProjectAxe();
		StartSpinAxe();
		Player->bAxeThrown = true;

	}
}

void ALeviathanAxe::SpinAxe(float RotateScalar)
{
		
	float RotationY = RotateScalar*-360.0f;
	FRotator newRotator = FRotator(RotationY,0.f,0.f);
	CenterPoint->SetRelativeRotation(newRotator);
			
}

void ALeviathanAxe::StopAxeMovement()
{
	ThrowParticles->EndTrails();
	StopAxeTracing();
	ProjectileMovement->Deactivate();
}

void ALeviathanAxe::TimeoutTrace()
{
	StopAxeMovement();
	AxeMesh->SetVisibility(false);
}

void ALeviathanAxe::LodgeAxe(USoundBase* Sound,USoundBase* Sound2, USoundAttenuation* SoundAttenuation)
{
	//Play Sound (Arrays cannot be used as function parameters, this is why its like this)
	
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(),Sound,ImpactLocation,FRotator(0,0,0),
        1,1,0,SoundAttenuation);
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(),Sound2,ImpactLocation,FRotator(0,0,0),
        1,1,0,SoundAttenuation);
	
	StopAxeMovement();
	StopSpinAxe();
	
	CenterPoint->SetRelativeRotation(FRotator(0.0f,0.0f,0.0f));
	SetActorRotation(ThrowCameraRotator);
	
	//Make sure the lodge point is not reset 
	// FRotator Rotator = FRotator(CalculateImpactPitchOffset(),0.0f,RandomRollThrow);
	//
	// //Set the Randomized Rotation of the axe
	// LodgePoint->SetRelativeRotation(Rotator);

	
 
	//Adjust the impact location
	SetActorLocation(CalculateImpactLocation());
	//set the corresponding axe state
	AxeState = EAxeState::Lodged;
	
}

void ALeviathanAxe::SetupWiggleReturn(USoundBase* SoundAsset,USoundAttenuation* SoundAttenuation, ESetupEnum& OutputPin)
{
	//Player->bAxeRecalled = true;
	StopAxeTracing();
	AxeMesh->SetVisibility(true);
	ReturnSound_Ref = UGameplayStatics::SpawnSoundAttached(SoundAsset,AxeMesh,"",FVector(0,0,0),
		FRotator(0,0,0),EAttachLocation::SnapToTarget,false,
		0.0f,1.0f,0.0f,SoundAttenuation);
	
	//
	switch (AxeState)
	{
		case EAxeState::Launched:
			AxeZ_Offset = 10.f;
		//Set to execute this pin
		OutputPin = ESetupEnum::Launched;
		AxeState = EAxeState::Returning;
			break;
		case EAxeState::Lodged:
			//Setup output pin for calling wiggle function
			OutputPin = ESetupEnum::Lodged;
			break;
			
	}
	
}

void ALeviathanAxe::SetupTimelineReturn()
{
	Player->bAxeRecalled = true;
	//Get the difference between the Axe and the socket of the player mesh.
	FVector LocationVector = FVector(GetActorLocation() - Player->GetMesh()->GetSocketLocation(TEXT("AxeSocket")));
	//Clamp the distance so its not too far and convert to a float.
	float Distance = FMath::Clamp(LocationVector.Size(),0.0f,MaxDistanceCalculation);
	//Store for calculations later
	DistanceFromCharacter = Distance;
	//Prevents the axe traveling under ground by rising it up.
	PreventClippingOnReturn();
	//Setup al variables here
	InitialLocation = GetActorLocation();
	InitialRotator = GetActorRotation();
	InitialCameraRotator = Player->FollowCamera->GetComponentRotation();
	//Return Lodge Point rotation to normal for smoothly bringing the axe back
	LodgePoint->SetRelativeRotation(FRotator(0,0,0));
}

void ALeviathanAxe::WiggleAxe(float Rotation)
{
	//Get the Rotation of the Lodged Axe
	FRotator BaseRotator = BaseLodgedRotator;
	//Change rotation based on the timeline * the value of strength.
	BaseRotator.Roll += WiggleStrength*Rotation;
	LodgePoint->SetRelativeRotation(BaseRotator);
}

const float ALeviathanAxe::ReturnTimelineSpeed()
{
	float TimelineSpeed = 0.0f;
	TimelineSpeed = ReturnTimelineIdealDistance*ReturnSpeed;
	//Get the ratio based on the distance from the character
	TimelineSpeed /= DistanceFromCharacter;
	//Clamp the value for polish
	FMath::Clamp(TimelineSpeed,0.4f,7.5f);
	AxeReturnTimelineSpeed = TimelineSpeed;
	return TimelineSpeed;
}

float ALeviathanAxe::CalculateImpactPitchOffset()
{
	float InclinedSurfaceOffset = FMath::FRandRange(-30.0,-55.0);
	float FlatSurfaceOffset = FMath::FRandRange(-5.0f,-25.0f);
	FRotator Rotator = UKismetMathLibrary::MakeRotationFromAxes(ImpactNormal,FVector(0),FVector(0));

	//If its flat surface
	if(Rotator.Pitch > 0.0f)
	{
		return FlatSurfaceOffset - Rotator.Pitch;
	}
	return InclinedSurfaceOffset - Rotator.Pitch;

}

FVector ALeviathanAxe::CalculateImpactLocation()
{
	FRotator Rotator = UKismetMathLibrary::MakeRotationFromAxes(ImpactNormal,FVector(0,0,0),FVector(0,0,0));
	//If its flat surface
	if(Rotator.Pitch > 0.0f)
	{
		//Should make sure that the axe blade is facing the object that has impacted with
		AxeZ_Offset = (((90.f - Rotator.Pitch) / 90.f) *10.f);
		FVector CalculatedLocation = ImpactLocation + FVector(0,0,AxeZ_Offset);
		CalculatedLocation += GetActorLocation()-LodgePoint->GetComponentLocation();
		
		return CalculatedLocation;
	}

	//If pitch is not bigger than 0 then set to 0 and calculate
        AxeZ_Offset = 10.f;
		FVector CalculatedLocation = ImpactLocation + FVector(0,0,AxeZ_Offset);
		CalculatedLocation += GetActorLocation()-LodgePoint->GetComponentLocation();
		
		return CalculatedLocation;
	
	
	
}

void ALeviathanAxe::PreventClippingOnReturn()
{
	//Rise the axe byt AxeZReturnOffset.
	FVector AdjustedLocation = GetActorLocation();
	AdjustedLocation.Z += AxeZReturnOffset;
	SetActorLocation(AdjustedLocation);
}

bool ALeviathanAxe::ChangeGravityAndHit(float gravity)
{

	
	ProjectileMovement->ProjectileGravityScale = gravity;
	
	FVector Start = GetActorLocation()+FVector(0,0,41);
	FVector End = GetActorLocation()+FVector(0,0,41) + (GetActorRotation().Vector() * AxeTraceDistance);
	GetWorld()->LineTraceSingleByChannel(HitResult,Start,End,ECC_Visibility);

	// DrawDebugLine(GetWorld(),Start, End,FColor(255, 0, 0),false,
 //        7, 0,5);

	
	if(HitResult.bBlockingHit)
	{
		ImpactLocation = HitResult.ImpactPoint;
		ImpactNormal = HitResult.ImpactNormal;
		ESurfaceHit = UGameplayStatics::GetSurfaceType(HitResult);
		StopAxeTracing();
		return HitResult.bBlockingHit;
	}
	return HitResult.bBlockingHit;
}

void ALeviathanAxe::StartParticleTrail()
{
	ThrowParticles->BeginTrails(TEXT("BaseSocket"),TEXT("TipSocket"),
		ETrailWidthMode_FromCentre,1.0);
	//At this stage the Axe is finished wiggling and is returning
	AxeState = EAxeState::Returning;
}

void ALeviathanAxe::UpdateReturnAxePosition(float InitialAlphaRotation, float CloseAlphaRotation, float AxeCurvature,
	float Speed,float Volume)
{
	//Adjusts the curve based on distance from the character and a parameter to scale the curvature
	//Lower number = more curve
	float CurveLocation = (DistanceFromCharacter/AxeReturnCurveScalar)*AxeCurvature;
	//Get the vector to the right of the axe to apply the curvature
	FVector RightVector = Player->FollowCamera->GetRightVector()*CurveLocation;
	//Add the right Vector based on the location of the Axe Socket. This is why we use the Curve Timeline
	//This creates the curve by sending bigger and smaller multipliers
	FVector CalculatedCurveAxeLocation = Player->GetMesh()->GetSocketLocation(TEXT("AxeSocket"))+ RightVector;
	//Smoothly interpolate between the 2 locations, using the Speed parameter from the timeline.
	ReturnTargetLocation = FMath::Lerp(InitialLocation,CalculatedCurveAxeLocation,Speed);

	//Here calculate the Rotation (Axe changes tilt base on distance for polish effect, tilts more or less based on distance)
	FRotator StartingRotator = FMath::Lerp(InitialRotator,FRotator(InitialCameraRotator.Pitch,InitialCameraRotator.Yaw,ReturnAxeTilt),InitialAlphaRotation);
	//This rotator will start to blend with the Alpha once its closer to the AxeSocket, to adjust the axe to the hand of the player.
	FRotator FinalRotator = FMath::Lerp(StartingRotator,Player->GetMesh()->GetSocketRotation(TEXT("AxeSocket")),CloseAlphaRotation);
	
	
	//Tick the Actor Location and Rotation based on the Timeline
	SetActorLocationAndRotation(ReturnTargetLocation,FinalRotator);

	//Get stored reference to the sound spawned and increase the volume as it gets closer to player based on Timeline
	//Check if there's something in the pointer
	if(ReturnSound_Ref)
	{
		ReturnSound_Ref->SetVolumeMultiplier(Volume);
		
	}
	
}

float ALeviathanAxe::PlaySoundAndReturnAxeSpinTimelineRate(float TimelineRate)
{
	//Play all the sounds and stuff here.
	//If its moving forward stop the rotation timeline.
	StopSpinAxe();
	//Need to convert from seconds to Length for calculations
	float TimelineLength = 1.f/TimelineRate;
	//Offset the length
	float OffsettedLength = TimelineLength - ReturnSpinAxeStopDistanceOffset;
	//Calculate the number of spin needed based on the spin rate.
	NumberOfAxeSpins = FMath::RoundToInt(TimelineLength/ReturnAxeSpinRate);
	//Do the Length of the time line over the number of spins to get the length of the spins.
	float SpinLength = OffsettedLength/NumberOfAxeSpins;
	//Convert the SpinLength back to TimeLine Play rate, just divide again by 1
	//Return the value (How fast the Timeline will play)
	return 1.f/SpinLength;
}

void ALeviathanAxe::DecreaseNumberOfSpins(ESpinsFunctionOutputEnum& OutputPin)
{
	//If there's one spin remaining its already playing, so don't play again or you will get an extra one.
	if(NumberOfAxeSpins == 1)
	{
		OutputPin = ESpinsFunctionOutputEnum::Done;
	}
	else
	{
		//Decrease number of spins and run again the timeline
		NumberOfAxeSpins--;
		OutputPin = ESpinsFunctionOutputEnum::Spinning;
	}
}


// Called every frame
void ALeviathanAxe::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ALeviathanAxe::MoveAxeToStartPosition()
{
	//Add the offset to the camera
	ThrowCameraLocation.X += SpinAxeAxisOffset;
	//Calculate new location
	FVector CalculatedLocation = ((ThrowDirection*AxeThrowScalar) + ThrowCameraLocation)
	- CenterPoint->GetRelativeLocation();
	this->SetActorLocationAndRotation(CalculatedLocation,ThrowCameraRotator);

	//Generate a random offset rotation when lodging the axe (more realistic touch)
	RandomRollThrow = FMath::FRandRange(5.0,-5.0);
	FRotator Rotator = FRotator(CalculateImpactPitchOffset(),0.0f,RandomRollThrow);

	//Set the Randomized Rotation of the axe
	LodgePoint->SetRelativeRotation(Rotator);
	//Store the rotator for later on.
	BaseLodgedRotator = Rotator;

	
}

void ALeviathanAxe::ProjectAxe()
{
	//Set Projectile velocity of the axe now that is detached
	ProjectileMovement->Velocity = ThrowDirection * ThrowSpeed;
	//Activate Projectile movement
	ProjectileMovement->Activate();
	//Set enum to launched axe
	AxeState = EAxeState::Launched;
	//Start fancy particle effect trail
	ThrowParticles->BeginTrails("BaseSocket","TipSocket",ETrailWidthMode_FromCentre
		,1.0f);
	//Remove gravity to simulate axe thrown very hard
	ProjectileMovement->ProjectileGravityScale = 0.0f;
	
}
