// Fill out your copyright notice in the Description page of Project Settings.


#include "Grabber.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

// Sets default values for this component's properties
UGrabber::UGrabber()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UGrabber::BeginPlay()
{
	Super::BeginPlay();
	
}


// Called every frame
void UGrabber::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UPhysicsHandleComponent* PhysicsHandle = GetPhysicsHandle();

	if(PhysicsHandle && PhysicsHandle->GetGrabbedComponent() != nullptr)
	{
		PhysicsHandle->SetTargetLocationAndRotation(
        		GetComponentLocation() + GetForwardVector() * HoldDistance,
        		GetComponentRotation());
	}
}

void UGrabber::Grab()
{
	UPhysicsHandleComponent* PhysicsHandle = GetPhysicsHandle();
	if(PhysicsHandle == nullptr)
	{
		return;
	}
	
	FHitResult hitResult;
	if(GetGrabbableInReach(hitResult))
	{
		UPrimitiveComponent* HitComponent = hitResult.GetComponent();
		HitComponent->SetSimulatePhysics(true);
		HitComponent->WakeAllRigidBodies();
		hitResult.GetActor()->Tags.Add("Grabbed");
		hitResult.GetActor()->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		PhysicsHandle->GrabComponentAtLocationWithRotation(
			HitComponent, NAME_None,
			hitResult.ImpactPoint,
			GetComponentRotation());
		//UE_LOG(LogTemp, Display, TEXT("Hit %s"), *HitResult.GetActor()->GetName());
	}
	else
	{
		//UE_LOG(LogTemp, Display, TEXT("No Actor Hit"));
	}
}

void UGrabber::Release()
{
	UPhysicsHandleComponent* PhysicsHandle = GetPhysicsHandle();

	if(PhysicsHandle && PhysicsHandle->GetGrabbedComponent() != nullptr)
	{
		PhysicsHandle->GetGrabbedComponent()->WakeAllRigidBodies();
		PhysicsHandle->GetGrabbedComponent()->GetOwner()->Tags.Remove("Grabbed");
		PhysicsHandle->ReleaseComponent();
	}
}

AActor* UGrabber::GrabbedActor() const
{
	UPhysicsHandleComponent* PhysicsHandle = GetPhysicsHandle();

	if(PhysicsHandle && PhysicsHandle->GetGrabbedComponent() != nullptr)
	{
		return PhysicsHandle->GetGrabbedComponent()->GetOwner();
	}
	return nullptr;
}

UPhysicsHandleComponent* UGrabber::GetPhysicsHandle() const
{
	UPhysicsHandleComponent* PhysicsHandle = GetOwner()->FindComponentByClass<UPhysicsHandleComponent>();
	if(PhysicsHandle == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Grabber requires a Physics Handle"));
		return nullptr;
	}

	return PhysicsHandle;
}

bool UGrabber::GetGrabbableInReach(FHitResult& OutHitResult) const
{
	FVector Start = GetComponentLocation();
	FVector End = Start + GetForwardVector() * MaxGrabDistance;
	DrawDebugLine(GetWorld(), Start, End, FColor::Red);

	FCollisionShape Sphere = FCollisionShape::MakeSphere(GrabRadius);
	return GetWorld()->SweepSingleByChannel(
		OutHitResult, Start, End, FQuat::Identity, 
		ECC_GameTraceChannel2, Sphere);
}
