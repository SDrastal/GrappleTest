// Fill out your copyright notice in the Description page of Project Settings.


#include "GrapplingHook.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

// Sets default values for this component's properties
UGrapplingHook::UGrapplingHook()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

//Defining start and stop functions that are used in the player blueprint
void UGrapplingHook::StartGrapple(EGrappleMode Mode)
{
	CurrentMode = Mode;
	FireHook();
}

void UGrapplingHook::StopGrapple()
{
	//If the player stops grappling, then grappling is false and they are not in a grapple mode (obviously)
	bIsGrappling = false;
	CurrentMode = EGrappleMode::None;

	//Makes sure there's a player to get movement component from
	if (!PlayerCharacter) return;

	//Gets the player's movement to return it to walking
	auto* Movement = PlayerCharacter->GetCharacterMovement();
	if (!Movement) return;

	//The pull makes the player fly, this makes them walk when finished
	Movement->SetMovementMode(MOVE_Walking);
}

//Function to fire the grapple, the point obtained from this is used in both the pull and swing functions
void UGrapplingHook::FireHook()
{
	//Make sure there's a player to get location/rotation from
	PlayerCharacter = Cast<ACharacter>(GetOwner());
	if (!PlayerCharacter) return;
	
	//Defines player's orientation when firing the grapple
	FVector Start;
	FRotator Rotation;
	
	//Grapple fires from the center of player's camera, this sees where they are looking on a fired grapple
	PlayerCharacter->GetActorEyesViewPoint(Start, Rotation);
	
	//Defines grapple's end point, or nothing if there is nothing hit
	FVector End = Start + (Rotation.Vector() * MaxDistance);
	
	//Gets the first object (not the player) hit by the grapple
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(PlayerCharacter);
	
	//Traces a line between the points of the grapple, if it hits anything then bHit is true
	bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit,
		Start,
		End,
		ECC_Visibility,
		Params
		);
	
	//Checks if the hook actually hits something or not
	if (bHit)
	{
		GrapplePoint = Hit.ImpactPoint;
		bIsGrappling = true;
	}
	else
	{
		StopGrapple();
	}
}

//Pull function that uses the grapple point from the hook, and pulls the player towards it
void UGrapplingHook::HandlePull(float DeltaTime)
{
	//Safety checks to make sure there is a player character and that the player's grappling
	if (!PlayerCharacter) return;
	if (!bIsGrappling) return;

	//Gets the player's location and the direction to the grapple point in relation to the player
	FVector PlayerLocation = PlayerCharacter->GetActorLocation();
	FVector ToTarget = GrapplePoint - PlayerLocation;

	//Distance and direction to the grapple point, used for calculating the pull
	float Distance = ToTarget.Size();
	FVector Direction = ToTarget.GetSafeNormal();

	//Gets the player's movement to apply the pull
	auto* Movement = PlayerCharacter->GetCharacterMovement();
	if (!Movement) return;

	//Gets the radius of the player's capsule component
	float CapsuleRadius = PlayerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();

	//Distance from the grapple point where the player will stop being pulled, using the capsule to prevent hitbox issues
	float SafeStopDistance = StopDistance + CapsuleRadius;

	//Check to make sure the player isn't too close to the object
	if (Distance <= SafeStopDistance)
	{
		//Stops player movement when too close
		Movement->Velocity = FVector::ZeroVector;

		//Gets the closest point to the grapple point while still being outside the player's capsule
		FVector SafeLocation = GrapplePoint - Direction * SafeStopDistance;

		//Sets player's location to that safe point
		PlayerCharacter->SetActorLocation(SafeLocation);

		//Makes the player fly to simulate "anchoring" to the pulled point
		Movement->SetMovementMode(MOVE_Flying);
		return;
	}

	//Slowdown for the player reaching the slowdown distance, so stopping isn't just instant
	float SpeedFactor = 1.0f;

	//Smoothing strength that'll be used by interpolating the player's velocity a bit later
	float SmoothStrength = 5.0f;

	//If the player's in the slowdown distance, make it so they start to slow down while still approaching the grapple point
	if (Distance < SlowDownDistance)
	{
		SpeedFactor = Distance / SlowDownDistance;
		Movement->SetMovementMode(MOVE_Flying);
	}

	//Target speed is the max pull speed multiplied by the slowdown factor, so the player slows down as they get closer to the grapple point
	float TargetSpeed = MaxPullSpeed * SpeedFactor;

	//Desired velocity gets how fast the player should move to the grapple point
	FVector DesiredVelocity = Direction * TargetSpeed;

	//Smoothly interpolate the player's current velocity towards the desired velocity, so the pull isn't just instant and jarring
	FVector NewVelocity = FMath::VInterpTo(
		Movement->Velocity,
		DesiredVelocity,
		DeltaTime,
		SmoothStrength
	);

	//Moves the player towards the grapple point with the new velocity
	Movement->Velocity = NewVelocity;
}

//Swing function that uses the grapple point from the hook, and allows the player to swing around it
void UGrapplingHook::HandleSwing(float DeltaTime)
{
	//Another check for if there's a player for the actor component or not
	if (!PlayerCharacter) return;
	
	//Gets player's location and distance the player is from the anchor, used later for calculating swinging
	FVector PlayerLocation = PlayerCharacter->GetActorLocation();
	FVector ToAnchor = PlayerLocation - GrapplePoint;

	//Current length of the rope for constraining the player to the grapple point and calculating the swing
	float CurrentLength = ToAnchor.Size();
	float MaxLength = 2000.f;

	//Gets player movement for applying swinging forces
	auto* Movement = PlayerCharacter->GetCharacterMovement();
	if (!Movement) return;
	
	//Constrain to max rope length so the player is actually tethered to where they grapple
	if (CurrentLength > MaxLength)
	{
		FVector Direction = ToAnchor / CurrentLength;
		FVector ClampedPosition = GrapplePoint + Direction * MaxLength;
		PlayerCharacter->SetActorLocation(ClampedPosition);
	}

	//Removes any potential velocity towards the anchor point so the player can only swing around it
	FVector Velocity = Movement->Velocity;
	FVector RopeDir = ToAnchor / CurrentLength;
	FVector TangentVelocity = Velocity - FVector::DotProduct(Velocity, RopeDir) * RopeDir;
	Movement->Velocity = TangentVelocity;
}

// Called when the game starts
void UGrapplingHook::BeginPlay()
{
	Super::BeginPlay();
	
}


// Called every frame
void UGrapplingHook::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//Won't constantly run unless the player is actively grappling
	if (!bIsGrappling) return;
	
	//Going between the modes that the player is in depending on if they are swinging or pulling
	switch (CurrentMode)
	{
		case EGrappleMode::Pull:
			HandlePull(DeltaTime);
			break;
		
		case EGrappleMode::Swing:
			HandleSwing(DeltaTime);
			break;
		
	default:
		break;
	}
}

