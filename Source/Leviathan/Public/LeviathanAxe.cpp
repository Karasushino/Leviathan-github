// Fill out your copyright notice in the Description page of Project Settings.

#include "LeviathanAxe.h"
#include "LeviathanCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"

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
	Current = 0.0f;

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
