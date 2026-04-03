// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GrapplingHook.generated.h"

// Since swinging and pulling are seperate but share the same code, this helps differentiate the two
UENUM(BlueprintType)
enum class EGrappleMode : uint8
{
	None,
	Pull,
	Swing
};

class ACharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GRAPPLETEST_API UGrapplingHook : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGrapplingHook();
	
	//Functions to start and stop grapple, used in the Player blueprint
	UFUNCTION(BlueprintCallable, Category="Grapple")
	void StartGrapple(EGrappleMode Mode);

	UFUNCTION(BlueprintCallable, Category="Grapple")
	void StopGrapple();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	//Definining functions that are in the cpp file
	void FireHook();
	void HandlePull(float DeltaTime);
	void HandleSwing(float DeltaTime);
	
	//The point the player is pulled to when pulling, or the anchor point for swinging
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grapple", meta = (AllowPrivateAccess = "true"))
	FVector GrapplePoint;

	//Bool checking if the player is grappling, prevents the cpp's tick from constantly running
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grapple", meta = (AllowPrivateAccess = "true"))
	bool bIsGrappling = false;
	
	//Player starts out not grappling by default
	EGrappleMode CurrentMode = EGrappleMode::None;
	
	//Maximum distance that the grapples can go (if they hit nothing then they do nothing)
	UPROPERTY(EditAnywhere, Category = "Grapple")
	float MaxDistance = 2000.f;

	//Accleration applied to the player when being pulled
	UPROPERTY(EditAnywhere, Category = "Grapple")
	float PullAcceleration = 6000.f;
	
	//Max speed the player's pulled, prevents the player from breaking the sound barrier
	UPROPERTY(EditAnywhere, Category = "Grapple")
	float MaxPullSpeed = 2000.f;

	//Distance where the player will begin to slow down if they're close enough to the object being pulled
	UPROPERTY(EditAnywhere, Category = "Grapple")
	float SlowDownDistance = 400.f;
	
	//Distance where pulling will stop if the player is close enough to the tethered object
	UPROPERTY(EditAnywhere, Category = "Grapple")
	float StopDistance = 25.f;
	
	//Get the address of the player when connected to be used in the cpp file
	UPROPERTY()
	ACharacter* PlayerCharacter;
		
};
