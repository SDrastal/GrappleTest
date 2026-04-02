// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GrapplingHook.generated.h"

UENUM(BlueprintType)
enum class EGrappleMode : uint8
{
	None,
	Pull,
	Swing
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GRAPPLETEST_API UGrapplingHook : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGrapplingHook();
	
	void StartGrapple(EGrappleMode Mode);
	void StopGrapple();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void FireHook();
	void HandlePull(float DeltaTime);
	void HandleSwing(float DeltaTime);
	
	FVector GrapplePoint;
	bool bIsGrappling;
	
	EGrappleMode CurrentMode;
	
	//Maximum distance that the grapples can go (if they hit nothing then they stop/"despawn")
	UPROPERTY(EditAnywhere)
	float MaxDistance = 2000.f;
	
	//The speed that the pulling grapple pulls the player
	UPROPERTY(EditAnywhere)
	float PullSpeed = 1000.f;
	
	//Distance "limit" for stopping the player if they're close enough to their pulling point
	UPROPERTY(EditAnywhere)
	float StopDistance = 100.f;
	
	//Gets the player that the actor component is connected to
	ACharacter* PlayerCharacter;
		
};
