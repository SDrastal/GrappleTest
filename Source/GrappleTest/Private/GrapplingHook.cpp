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

void UGrapplingHook::StartGrapple(EGrappleMode Mode)
{
	CurrentMode = Mode;
	FireHook();
}

void UGrapplingHook::StopGrapple()
{
	bIsGrappling = false;
	CurrentMode = EGrappleMode::None;

	if (!PlayerCharacter) return;

	auto* Movement = PlayerCharacter->GetCharacterMovement();
	if (!Movement) return;

	// Stop ALL movement immediately
	Movement->StopMovementImmediately();

	// Reset mode
	Movement->SetMovementMode(MOVE_Walking);
}

void UGrapplingHook::FireHook()
{
	PlayerCharacter = Cast<ACharacter>(GetOwner());
	if (!PlayerCharacter) return;
	
	FVector Start;
	FRotator Rotation;
	
	PlayerCharacter->GetActorEyesViewPoint(Start, Rotation);
	
	FVector End = Start + (Rotation.Vector() * MaxDistance);
	
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(PlayerCharacter);
	
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

void UGrapplingHook::HandlePull(float DeltaTime)
{
	if (!PlayerCharacter) return;

	if (!bIsGrappling) return;

	FVector PlayerLocation = PlayerCharacter->GetActorLocation();
	FVector ToTarget = GrapplePoint - PlayerLocation;

	float Distance = ToTarget.Size();
	FVector Direction = ToTarget.GetSafeNormal();

	auto* Movement = PlayerCharacter->GetCharacterMovement();
	if (!Movement) return;

	// If close enough STOP and HOLD
	float CapsuleRadius = PlayerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();

	float SafeStopDistance = StopDistance + CapsuleRadius;

	if (Distance <= SafeStopDistance)
	{
		Movement->Velocity = FVector::ZeroVector;

		// Stop slightly BEFORE the wall
		FVector SafeLocation = GrapplePoint - Direction * SafeStopDistance;

		PlayerCharacter->SetActorLocation(SafeLocation);

		Movement->SetMovementMode(MOVE_Flying);
		return;
	}

	// Smooth slowdown when approaching target
	float SpeedFactor = 1.0f;

	if (Distance < SlowDownDistance)
	{
		SpeedFactor = Distance / SlowDownDistance; // 0  1
		Movement->SetMovementMode(MOVE_Flying);
	}

	float TargetSpeed = MaxPullSpeed * SpeedFactor;

	FVector DesiredVelocity = Direction * TargetSpeed;

	// Smooth interpolation instead of snapping
	FVector NewVelocity = FMath::VInterpTo(
		Movement->Velocity,
		DesiredVelocity,
		DeltaTime,
		5.0f // smoothing strength
	);

	Movement->Velocity = NewVelocity;
}

void UGrapplingHook::HandleSwing(float DeltaTime)
{
	//Another check for if there's a player for the actor component or not
	if (!PlayerCharacter) return;
	
	FVector PlayerLocation = PlayerCharacter->GetActorLocation().GetSafeNormal();
	
	FVector ToAnchor = PlayerLocation - GrapplePoint;
	
	float CurrentLength = ToAnchor.Size();
	
	FVector Direction = ToAnchor.GetSafeNormal();
	
	float MaxLength = 2000.f;
	
	if (CurrentLength > MaxLength)
	{
		FVector ClampedPosition = GrapplePoint + Direction * MaxLength;
		
		FVector Correction = ClampedPosition - PlayerLocation;
		
		PlayerCharacter->SetActorLocation(PlayerLocation + Correction);
	}
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

