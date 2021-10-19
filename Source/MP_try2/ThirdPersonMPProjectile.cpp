// Fill out your copyright notice in the Description page of Project Settings.


#include "ThirdPersonMPProjectile.h"

#include "MP_try2Character.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/DamageType.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values
AThirdPersonMPProjectile::AThirdPersonMPProjectile()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	//Definition for the SphereComponent that will serve as the Root component for the projectile and its collision.
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("RootComponent"));
	SphereComponent->InitSphereRadius(10.f);
	SphereComponent->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	RootComponent = SphereComponent;

	//Registering the Projectile Impact function on a Hit event.
	if (GetLocalRole() == ROLE_Authority)
	{
		SphereComponent->OnComponentHit.AddDynamic(this, &AThirdPersonMPProjectile::OnProjectileImpact);
	}

	//Definition for the Mesh that will serve as our visual representation.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(
		TEXT("/Game/StarterContent/Shapes/Shape_Sphere.Shape_Sphere"));
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	StaticMesh->SetupAttachment(RootComponent);

	//Set the Static Mesh and its position/scale if we successfully found a mesh asset to use.
	if (DefaultMesh.Succeeded())
	{
		StaticMesh->SetStaticMesh(DefaultMesh.Object);
		StaticMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -37.5f));
		StaticMesh->SetRelativeScale3D(FVector(0.75f, 0.75f, 0.75f));
	}

	static ConstructorHelpers::FObjectFinder<UParticleSystem> DefaultExplosionEffect(
		TEXT("/Game/StarterContent/Particles/P_Explosion.P_Explosion"));
	if (DefaultExplosionEffect.Succeeded())
	{
		ExplosionEffect = DefaultExplosionEffect.Object;
	}

	//Definition for the Projectile Movement Component.
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovementComponent->SetUpdatedComponent(SphereComponent);
	ProjectileMovementComponent->InitialSpeed = 1500.0f;
	ProjectileMovementComponent->MaxSpeed = 1500.0f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->ProjectileGravityScale = 0.0f;

	DamageType = UDamageType::StaticClass();
	Damage = 10.0f;
}

// Called when the game starts or when spawned
void AThirdPersonMPProjectile::BeginPlay()
{
	Super::BeginPlay();

	// set homing target
	//if (GetLocalRole() == ROLE_Authority)
	{
		TArray<AActor*> FoundCharacters;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMP_try2Character::StaticClass(), FoundCharacters);
		UE_LOG(LogTemp, Log, TEXT("FoundCharacters %i"), FoundCharacters.Num());

		if (FoundCharacters.Num() > 1)
		{
			//GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, FString::Printf(TEXT("projectile: %s"), *GetName()));
			const float Dist0 = FVector::Distance(GetActorLocation(), FoundCharacters[0]->GetActorLocation());
			const float Dist1 = FVector::Distance(GetActorLocation(), FoundCharacters[1]->GetActorLocation());
			AActor* FurtherCharacter =  Dist0 > Dist1 ? FoundCharacters[0] : FoundCharacters[1];
			ProjectileMovementComponent->bIsHomingProjectile = true;
			UE_LOG(LogTemp, Log, TEXT("set HomingTargetComponent to %s"), *FurtherCharacter->GetName());
			ProjectileMovementComponent->HomingTargetComponent =
				Cast<USceneComponent>(Cast<AMP_try2Character>(FurtherCharacter)->ProjectileTarget);
		}
	}
}

void AThirdPersonMPProjectile::Destroyed()
{
	UE_LOG(LogTemp, Log, TEXT("Destroyed"));

	FVector spawnLocation = GetActorLocation();
	UGameplayStatics::SpawnEmitterAtLocation(this, ExplosionEffect, spawnLocation, FRotator::ZeroRotator, true,
	                                         EPSCPoolMethod::AutoRelease);
}

void AThirdPersonMPProjectile::OnProjectileImpact(UPrimitiveComponent* HitComponent, AActor* OtherActor,
                                                  UPrimitiveComponent* OtherComp, FVector NormalImpulse,
                                                  const FHitResult& Hit)
{
	if (OtherActor)
	{
		UE_LOG(LogTemp, Log, TEXT("OnProjectileImpact"));
		UGameplayStatics::ApplyPointDamage(OtherActor, Damage, NormalImpulse, Hit, GetInstigator()->Controller, this,
		                                   DamageType);

		FHitResult OutHit;
		UKismetSystemLibrary::SphereTraceSingleForObjects(GetWorld(), GetTransform().GetLocation(),
		                                                  GetTransform().GetLocation(), 100,
		                                                  ObjectTypesToDebug, false,
		                                                  TArray<AActor*>(), EDrawDebugTrace::Type::ForDuration, OutHit,
		                                                  true, FLinearColor::Red, FLinearColor::Green, 10);

		if (OutHit.Actor != nullptr)
			UE_LOG(LogTemp, Log, TEXT("hit %s"), *OutHit.Actor->GetName());

		OnProjectileImpactBP();
	}

	Destroy();
}

// Called every frame
void AThirdPersonMPProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
