// Fill out your copyright notice in the Description page of Project Settings.

#include "LeviathanAxe.h"

#include "DrawDebugHelpers.h"
#include "LeviathanCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
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
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Throw was called"));
		//Detach the Axe from the player
		this->DetachFromActor(FDetachmentTransformRules(EDetachmentRule::KeepWorld,true));
		//
		//Get all Data for maths
		ThrowCameraRotator = Player->FollowCamera->GetComponentRotation();
		ThrowDirection = Player->FollowCamera->GetForwardVector();
		ThrowCameraLocation = Player->FollowCamera->GetComponentLocation();
		ThrowSpeed = 2500.f;
		
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
	
	//Generate a random offset rotation when lodging the axe (more realistic touch)
	float Roll = FMath::FRandRange(-3.0,-8.0);
	FRotator Rotator = FRotator(CalculateImpactPitchOffset(),0.0f,Roll);

	//Set the Randomized Rotation of the axe
	LodgePoint->SetRelativeRotation(Rotator);

	
 
	//Adjust the impact location
	SetActorLocation(CalculateImpactLocation());
	//set the corresponding axe state
	AxeState = EAxeState::Lodged;
	
	
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

bool ALeviathanAxe::ChangeGravityAndHit(float gravity)
{

	
	ProjectileMovement->ProjectileGravityScale = gravity;
	
	FVector Start = GetActorLocation()+FVector(0,0,41);
	FVector End = GetActorLocation()+FVector(0,0,41) + (GetActorRotation().Vector() * AxeTraceDistance);
	GetWorld()->LineTraceSingleByChannel(HitResult,Start,End,ECC_Visibility);

	DrawDebugLine(GetWorld(),Start, End,FColor(255, 0, 0),false,
        7, 0,5);

	
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
