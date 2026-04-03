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

class ACharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GRAPPLETEST_API UGrapplingHook : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGrapplingHook();
	
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
	void FireHook();
	void HandlePull(float DeltaTime);
	void HandleSwing(float DeltaTime);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grapple", meta = (AllowPrivateAccess = "true"))
	FVector GrapplePoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grapple", meta = (AllowPrivateAccess = "true"))
	bool bIsGrappling = false;
	
	EGrappleMode CurrentMode = EGrappleMode::None;
	
	//Maximum distance that the grapples can go (if they hit nothing then they stop/"despawn")
	UPROPERTY(EditAnywhere, Category = "Grapple")
	float MaxDistance = 2000.f;

	UPROPERTY(EditAnywhere, Category = "Grapple")
	float PullAcceleration = 6000.f;
	
	UPROPERTY(EditAnywhere, Category = "Grapple")
	float MaxPullSpeed = 2000.f;

	UPROPERTY(EditAnywhere, Category = "Grapple")
	float SlowDownDistance = 400.f;

	//The speed that the pulling grapple pulls the player
	UPROPERTY(EditAnywhere, Category = "Grapple")
	float PullSpeed = 1000.f;
	
	//Distance "limit" for stopping the player if they're close enough to their pulling point
	UPROPERTY(EditAnywhere, Category = "Grapple")
	float StopDistance = 100.f;
	
	//Get the address of the player when connected
	UPROPERTY()
	ACharacter* PlayerCharacter;
		
};
