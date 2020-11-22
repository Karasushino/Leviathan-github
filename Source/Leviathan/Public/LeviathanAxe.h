// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/EngineTypes.h"
#include "Particles/ParticleSystemComponent.h"
#include "UObject/ObjectMacros.h"

#include "LeviathanAxe.generated.h"

//Enum for output pins
UENUM(BlueprintType)
enum class ESetupEnum : uint8 {Launched,Lodged};
UENUM(BlueprintType)
enum class ESpinsFunctionOutputEnum : uint8 {Spinning,Done};


UENUM()
enum class EAxeState {Idle,Launched,Lodged,Returning };
UCLASS()
class LEVIATHAN_API ALeviathanAxe : public AActor
{
	GENERATED_BODY()
	
	
public:	
	// Sets default values for this actor's properties
	ALeviathanAxe();
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Axe)
	class ALeviathanCharacter* Player;
	


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	//Ref to main character

	
	void MoveAxeToStartPosition();
	void ProjectAxe();
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

//Components for Axe
#pragma region AxeComponents
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	class USceneComponent* Root;
	UPROPERTY(BlueprintReadWrite,EditAnywhere)
	class USceneComponent* CenterPoint;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	class USceneComponent* LodgePoint;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	class USkeletalMeshComponent* AxeMesh;
#pragma endregion 
//Component for projectile calculations
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	class UProjectileMovementComponent* ProjectileMovement;

//Particle Components
#pragma region ParticleComponents
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=ParticleSystem)
	class UParticleSystemComponent* ThrowParticles;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=ParticleSystem)
	class UParticleSystemComponent* SwingParticles;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=ParticleSystem)
	class UParticleSystemComponent* AxeCatchParticles;
#pragma endregion 

	
#pragma region Axe Variables
//Distance from Player (Axe from player) when recalled
UPROPERTY(BlueprintReadWrite, EditAnywhere)
float DistanceFromCharacter;
//Initial Location of Axe when Recalled
UPROPERTY(BlueprintReadWrite, EditAnywhere)
FVector InitialLocation;
//Initial Rotation of Axe when Recalled
UPROPERTY(BlueprintReadWrite, EditAnywhere)
FRotator InitialRotator;
//Initial Camera Rotator when Recalled
FRotator InitialCameraRotator;
//Enum for the AxeState 
UPROPERTY(BlueprintReadWrite, EditAnywhere)
EAxeState AxeState;
//Vector to store the Impact Location of the Axe
UPROPERTY(BlueprintReadWrite, EditAnywhere)
FVector ImpactLocation;
//The rate the Axe spins at
UPROPERTY(BlueprintReadWrite, EditAnywhere)
float AxeSpinRate;
//The Forward directional vector from the camera of the player.
UPROPERTY(BlueprintReadWrite, EditAnywhere)
FVector ThrowDirection;
//The Location of camera of the player when thrown. (Frame when thrown)
UPROPERTY(BlueprintReadWrite, EditAnywhere)
FVector ThrowCameraLocation;
//The Rotator of the camera of the player when thrown. (Frame when thrown)
UPROPERTY(BlueprintReadWrite, EditAnywhere)
FRotator ThrowCameraRotator;
/**The optimal distance for the timeline to work as set up (default). Timeline is later on scaled to fit
the distance, thus making it dynamic*/
UPROPERTY(BlueprintReadWrite, EditAnywhere)
float OptimalDistance;
//Get the base rotator for the Blade point, the Lodge point, the point where it will be attached.
UPROPERTY(BlueprintReadWrite, EditAnywhere)
FRotator LodgePointBaseRotator;
//Set the impulse strength of the axe throw
UPROPERTY(BlueprintReadWrite, EditAnywhere)
float AxeImpulseStrength;
//Offset for the spin axis of the axe. (Tilts the axe, so its thrown side ways for example)
//This has potential to be used to change this code from an axe throw to a shield throw or other weapons.
UPROPERTY(BlueprintReadWrite, EditAnywhere)
float SpinAxeAxisOffset;
//All of this is to be used to the Projectile movement component
//Speed of the axe when thrown
UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AxeSettings")
float ThrowSpeed = 2500.f;
//Return Speed of the axe
UPROPERTY(BlueprintReadWrite, EditAnywhere)
float ReturnSpeed = 1.0f;
//Impact normal. Store the normal when impact with something, out of the Break Hit Result node.
UPROPERTY(BlueprintReadWrite, EditAnywhere)
FVector ImpactNormal;
//Stores the Bone name of where the impact happened, out of the Break Hit Result node.
UPROPERTY(BlueprintReadWrite)
FName BoneHitName;
UPROPERTY(EditDefaultsOnly)
float AxeTraceDistance = 60.f;
//Store the hit result of the linetracebychannel
UPROPERTY(BlueprintReadOnly)
FHitResult HitResult;
UPROPERTY(BlueprintReadOnly)
TEnumAsByte<EPhysicalSurface> ESurfaceHit;
UPROPERTY(BlueprintReadOnly)
bool bHitBlocked;

//Used for tracing the path back using (SphereTraceByChannel)
//Vector for the Target Location (Axe Location Last Tick)
UPROPERTY(BlueprintReadWrite, EditAnywhere)
FVector PreviousAxeLocation;
//Vector for the Current Target Location (Axe Location This Tick)
UPROPERTY(BlueprintReadWrite, EditAnywhere)
FVector CurrentAxeLocation;
//The number of spins to do when returning (This number changes based on the distance from the axe to the player)
int NumberOfAxeSpins;
//Float to set the maximum distance that the axe will do the calculation from (prevents from the axe to take
//forever to return if it goes too far).
UPROPERTY(BlueprintReadWrite, EditAnywhere)
float MaxDistanceCalculation = 3000.f;
//The tilt on the X-axis of the Axe when returning, how sideways is it going to return.
UPROPERTY(BlueprintReadWrite, EditAnywhere)
float ReturnAxeTilt = 60.f;
//The rate that the Axe is going to speed at when returning to the hand.
UPROPERTY(BlueprintReadWrite, EditAnywhere)
float ReturnAxeSpinRate;
//Fine tuning variable, considering removing.
/**Z-Adjustment. Changes how high or low the axe will return from the player. Higher number makes it lower.
Prevents the axe to clip through the ground if it landed in the ground, it rises it a bit so you can see it coming
Back to you.*/
UPROPERTY(BlueprintReadWrite, EditAnywhere)
float AxeZ_Offset;
/**Still don't fully understand how its used but understand what it does, look this further in unreal documentation
 * LineTraceByChannel, and what Start and End is. Projectile_Leviathon>EventGraph
 */
UPROPERTY(BlueprintReadWrite, EditAnywhere)
float ThrowAxeLineTraceDistance;
//Store the enum provided from the Hit result to decide what sound to play.
UPROPERTY(BlueprintReadWrite, EditAnywhere)
TEnumAsByte<EPhysicalSurface> ImpactSurfaceType;
//Scalar value to make the axe curve more pronounced or less pronounced to the right.
UPROPERTY(BlueprintReadWrite, EditAnywhere)
float AxeReturnCurveScalar = 1.f;
//Scalar value to make the snap the axe further or closer to the player when thrown.
UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AxeSettings")
float AxeThrowScalar = 250.f;
//Speed of the axe throw
UPROPERTY(BlueprintReadWrite, Category = "AxeSettings")
bool StopAxeRotation = false;

//Store the randomized roll rotation when axe is thrown.
float RandomRollThrow = 0.0f;
//Store the Base Lodged Rotator to use on Wiggle
FRotator BaseLodgedRotator;
//Wiggle Strength (Parameter used to multiply the timeline output value, therefore changing the rotation value)
UPROPERTY(EditAnywhere)
float WiggleStrength = 12.f;
//The number of units the axe will be risen up before returning to avoid clipping through the floor
UPROPERTY(EditAnywhere)
float AxeZReturnOffset = 50.f;
//Store a reference to the sound the axe makes when returning, to be able to edit it later.	
UAudioComponent* ReturnSound_Ref;

//Hardcoded Optimal Distance for the Return Timeline. (Will set a timeline based on this distance and then speed it up
//and slow it based on the difference)
float ReturnTimelineIdealDistance = 1400.f;
//Speed multiplier for the recall of the axe to player. 
FVector ReturnTargetLocation;
//Float to change the distance at where the axe will stop rotating. Lowest means closer to the player. 
UPROPERTY(EditAnywhere)
float ReturnSpinAxeStopDistanceOffset = 0.1f;

float AxeReturnTimelineSpeed;
#pragma endregion


	UFUNCTION(BlueprintImplementableEvent, Category = "ThrowAxe")
	void ThrowEvent();
	UFUNCTION(BlueprintImplementableEvent, Category = "RecallAxe")
    void RecallEvent();
	UFUNCTION(BlueprintCallable,Category = "ThrowAxe")
	void Throw();
	UFUNCTION(BlueprintCallable,Category = "ThrowAxe")
	void SpinAxe(float RotateScalar);
	UFUNCTION(BlueprintCallable, Category = "ThrowAxe")
    void StopAxeMovement();
	UFUNCTION(BlueprintCallable, Category = "ThrowAxe")
	void TimeoutTrace();
	UFUNCTION(BlueprintCallable, Category = "ThrowAxe")
    void LodgeAxe(USoundBase* SoundAsset,USoundBase* SoundAsset2,USoundAttenuation* AttenuationAsset);

	UFUNCTION(BlueprintCallable, Category = "ReturnAxe", Meta = (ExpandEnumAsExecs="OutputPin"))
	void SetupWiggleReturn(USoundBase* SoundAsset,USoundAttenuation* SoundAttenuation,ESetupEnum& OutputPin);
	//Mostly initialize variables and get everything ready
	UFUNCTION(BlueprintCallable, Category = "ReturnAxe")
    void SetupTimelineReturn();
	
	UFUNCTION(BlueprintCallable, Category = "ReturnAxe")
    void WiggleAxe(float Rotation);

	UFUNCTION(BlueprintPure, Category = "ReturnAxe") const
	float ReturnTimelineSpeed();
	
	UFUNCTION(BlueprintCallable, Category = "ParticlesAxe")
    void StartParticleTrail();
	
	UFUNCTION(BlueprintCallable, Category = "ReturnAxe")
	void UpdateReturnAxePosition(float InitialAlphaRotation, float CloseAlphaRotation, float AxeCurvature, float Speed,
		float Volume);

	UFUNCTION(BlueprintCallable, Category = "ReturnAxe")
	float PlaySoundAndReturnAxeSpinTimelineRate(float TimelineRate);
	UFUNCTION(BlueprintCallable, Category = "ReturnAxe",Meta = (ExpandEnumAsExecs="OutputPin"))
	void DecreaseNumberOfSpins(ESpinsFunctionOutputEnum& OutputPin);

	//All adjustments for polish and make the axe sticking to surfaces more realistic
	float CalculateImpactPitchOffset();
	FVector CalculateImpactLocation();
	void PreventClippingOnReturn();

	
	UFUNCTION(BlueprintCallable,Category="ThrowAxe")
	bool ChangeGravityAndHit(float gravity);
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "SpinAxe")
	void StartSpinAxe();
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "SpinAxe")
    void StopSpinAxe();
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "TraceAxe")
    void StopAxeTracing();
	
	
	
};
