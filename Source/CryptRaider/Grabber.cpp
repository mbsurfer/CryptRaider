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

	// FRotator Rotation = GetComponentRotation();
	// FString RotationString = Rotation.ToCompactString();
	// UE_LOG(LogTemp, Display, TEXT("%s"), *RotationString);

	// UWorld* World = GetWorld();
	// float ElapsedTime = World->TimeSeconds;
	// UE_LOG(LogTemp, Display, TEXT("Time PLayed: %f"), ElapsedTime);

	UPhysicsHandleComponent* PhysicsHandel = GetPhysicsHandel();

	// Only update the target when grabbing
	if (PhysicsHandel && PhysicsHandel->GetGrabbedComponent()) {
		PhysicsHandel->SetTargetLocationAndRotation(
			GetComponentLocation() + GetForwardVector() * HoldDistance, 
			GetComponentRotation()
		);
	}
}

void UGrabber::Grab()
{
	UPhysicsHandleComponent* PhysicsHandel = GetPhysicsHandel();
	FHitResult HitResult;

	if (PhysicsHandel && GetGrabbableInReach(HitResult)) {
		// FString TargetName = HitResult.GetActor()->GetActorNameOrLabel();
		// DrawDebugSphere(GetWorld(), HitResult.Location, 10, 10, FColor::Blue, false, 5);
		// DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 10, 10, FColor::Red, false, 5);
		// UE_LOG(LogTemp, Display, TEXT("Hit Target: %s"), *TargetName);

		// Make sure physics is enabled on that target component
		UPrimitiveComponent* HitComponent = HitResult.GetComponent();
		HitComponent->WakeAllRigidBodies();
		HitComponent->SetSimulatePhysics(true);

		GrabbedActor = HitResult.GetActor();

		// Inform the hit target that it is being held
		UGrabbable* GrabbableComponent = GetGrabbedComponent();
		if(!GrabbableComponent) {
			return;
		}

		GrabbedActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		GrabbableComponent->SetIsGrabbed(true);

		PhysicsHandel->GrabComponentAtLocationWithRotation(
			HitComponent, 
			NAME_None, 
			HitResult.ImpactPoint, 
			GetComponentRotation()
		);
	} else {
		// UE_LOG(LogTemp, Display, TEXT("No hits..."));
	}
}

void UGrabber::Release()
{
	UPhysicsHandleComponent* PhysicsHandel = GetPhysicsHandel();
	if (PhysicsHandel && PhysicsHandel->GetGrabbedComponent()) {
		GetGrabbedComponent()->SetIsGrabbed(false);
		PhysicsHandel->ReleaseComponent();
	}
}

UGrabbable* UGrabber::GetGrabbedComponent() const
{
	UGrabbable* GrabbableComponent = GrabbedActor->FindComponentByClass<UGrabbable>();
	if (!GrabbableComponent) {
		UE_LOG(LogTemp, Error, TEXT("Actor is missing GrabbableComponent: %s"), *GrabbedActor->GetActorNameOrLabel());
	}

	return GrabbableComponent;
}

UPhysicsHandleComponent* UGrabber::GetPhysicsHandel() const
{
	UPhysicsHandleComponent* PhysicsHandel = GetOwner()->FindComponentByClass<UPhysicsHandleComponent>();
	
	if (!PhysicsHandel) {
		UE_LOG(LogTemp, Error, TEXT("Grabber is missing a UPhysicsHandleComponent!"));
	}

	return  PhysicsHandel;
}

// Sweep the grabber channel and return true if we have a hit
bool UGrabber::GetGrabbableInReach(FHitResult& OutHitResult) const
{
	FVector Start = GetComponentLocation();
	FVector End = Start + GetForwardVector() * MaxGrabDistance;

	// DrawDebugLine(GetWorld(), Start, End, FColor::Green);
	// DrawDebugSphere(GetWorld(), End, 10, 10, FColor::Blue, false, 5);	

	FCollisionShape Sphere = FCollisionShape::MakeSphere(GrabRadius);
	return GetWorld()->SweepSingleByChannel(OutHitResult, Start, End, FQuat::Identity, GrabberChannel, Sphere);
}