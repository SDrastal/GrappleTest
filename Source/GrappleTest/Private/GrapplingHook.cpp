// Fill out your copyright notice in the Description page of Project Settings.


#include "GrapplingHook.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"

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
	//No player actor means the function does nothing
	if (!PlayerCharacter) return;
	
	FVector Direction = (GrapplePoint - PlayerCharacter->GetActorLocation().GetSafeNormal());
	FVector NewVelocity = Direction * PullSpeed;
	
	PlayerCharacter->GetCharacterMovement()->Velocity = NewVelocity;
	
	float Distance = FVector::Distance(PlayerCharacter->GetActorLocation(), GrapplePoint);
	
	//Stops pulling the player once they're close enough to the object
	if (Distance <= StopDistance)
	{
		StopGrapple();
	}
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

