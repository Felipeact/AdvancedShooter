// Fill out your copyright notice in the Description page of Project Settings.


#include "SooterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Animation/AnimMontage.h"
#include "DrawDebugHelpers.h"

// Sets default values
ASooterCharacter::ASooterCharacter() :
	BaseTurnRate(45.f),
	BaseLookUpRate(45.f),
	//Turn Rates for aiming / not aiming 
	HipTurnRate(90.f),
	HipLookUpRate(90.f),
	AimingTurnRate(20.f),
	AimingLookUpRate(20.f),
	// Mouse Look Sensitivity scale factors
	MouseHipTurnRate(1.0f),
	MouseHipLookUpRate(1.0f),
	MouseAimingTurnRate(0.2f),
	MouseAimingLookUpRate(0.2f),
	//true when aiming
	bAiming(false),
	// Camera field of view values
	CameraDefaultFOV(0.f), // Set in BeginPlay
	CameraZoomedFOV(60.f),
	CameraCurrentFov(0.f),
	ZoomInterpSpeed(20.f)
	
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//create a camera boom (pulls in towards character if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.f; // the camera follows at this distance behind the character 
	CameraBoom->bUsePawnControlRotation = true; // rotate the arm based on the controller 
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 50.f);

	//Create a Follow Camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attached Follow Camera to end of CameraBoom
	FollowCamera->bUsePawnControlRotation = false; //Camera does not rotate relative to arm

	// Dont rotate when the controller rotates. Let The controller only affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	//configure character movement 
	GetCharacterMovement()->bOrientRotationToMovement = false; // character moves in tehe direction of input
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);// ... at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;
}

// Called when the game starts or when spawned
void ASooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (FollowCamera)
	{
		CameraDefaultFOV = GetFollowCamera()->FieldOfView;
		CameraCurrentFov = CameraCurrentFov;
	}
	
}

void ASooterCharacter::MoveFoward(float Value)
{
	if (( Controller != nullptr) && ( Value != 0))
	{
		const FRotator Rotation{ Controller->GetControlRotation() };
		const FRotator YawRotation{ 0, Rotation.Yaw,0 };

		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::X)};
		
		AddMovementInput(Direction, Value);
		
	}
}

void ASooterCharacter::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0))
	{
		const FRotator Rotation{ Controller->GetControlRotation() };
		const FRotator YawRotation{ 0, Rotation.Yaw,0 };

		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::Y) };

		AddMovementInput(Direction, Value);

	}
}

void ASooterCharacter::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ASooterCharacter::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ASooterCharacter::Turn(float Value)
{	
	float TurnScaleFactor{};
	if (bAiming)
	{
		TurnScaleFactor = MouseAimingTurnRate;
	}
	else
	{
		TurnScaleFactor = MouseHipTurnRate;
	}

	AddControllerYawInput(Value * TurnScaleFactor);
}

void ASooterCharacter::LookUp(float Value)
{
	float LookUpScaleFactor{};
	if (bAiming)
	{
		LookUpScaleFactor = MouseAimingLookUpRate;
	}
	else
	{
		LookUpScaleFactor = MouseHipLookUpRate;
	}

	AddControllerPitchInput(Value * LookUpScaleFactor);
}

void ASooterCharacter::FireWeapon()
{
	if (FireSound)
	{
		UGameplayStatics::PlaySound2D(this, FireSound);
	}

	const USkeletalMeshSocket* BarrelSocket = GetMesh()->GetSocketByName("BarrelSocket");
	if (BarrelSocket)
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(GetMesh());

		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
		}

		FVector BeamEnd;
		bool bBeamEnd = GetBeamEndLocation(SocketTransform.GetLocation(), BeamEnd); //Function create before tick AA

		if (bBeamEnd)
		{
			//Spawn Impact after updatind beam end point
			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, BeamEnd);
			}

			if (BeamParticles)
			{
				UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransform);
				if (Beam)
				{
					Beam->SetVectorParameter(FName("Target"), BeamEnd);
				}
			}
		}


		/*FHitResult FireHit;
		const FVector Start{ SocketTransform.GetLocation()};
		const FQuat Rotation{ SocketTransform.GetRotation() };
		const FVector RotationAxis(Rotation.GetAxisX());
		const FVector End{ Start + RotationAxis * 50'000.f };

		FVector BeamEndPoint{ End };

		GetWorld()->LineTraceSingleByChannel(FireHit, Start, End, ECollisionChannel::ECC_Visibility );
		if (FireHit.bBlockingHit)
		{
			//DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 2.f);
			//DrawDebugPoint(GetWorld(), FireHit.Location, 5.f, FColor::Red, false, 2.f);
			
			BeamEndPoint = FireHit.Location;

			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, FireHit.Location);
			}
		}

		if (BeamParticles)
		{
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransform);
			if (Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamEndPoint);
			}
		}
		*/
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if( AnimInstance && HipFireMontage) 
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}
}

bool ASooterCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation)
{
	// Get Current size of viewport
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	//Get Screen space location of crosshair
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	CrosshairLocation.Y -= 50.f;

	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	//Get World position and direction of crosshairs
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation, CrosshairWorldPosition, CrosshairWorldDirection);

	if (bScreenToWorld) // was deprojection successful?
	{
		FHitResult ScreenTraceHit;
		const FVector Start{ CrosshairWorldPosition };
		const FVector End{ CrosshairWorldPosition + CrosshairWorldDirection * 50'000.f };


		// Set Beam end point to line trace and point
		OutBeamLocation = End;


		//trace outward from crosshairs world location
		GetWorld()->LineTraceSingleByChannel(ScreenTraceHit, Start, End, ECollisionChannel::ECC_Visibility);
		if (ScreenTraceHit.bBlockingHit) // was there a trace hit?
		{
			// Beam enf point is now trace hit location
			OutBeamLocation = ScreenTraceHit.Location;


		}

		// Perfotm a second trace, this time from the gun barrel 
		FHitResult WeaponTraceHit;
		const FVector WeaponTraceStart{ MuzzleSocketLocation };
		const FVector WeaponTraceEnd{ OutBeamLocation };
		GetWorld()->LineTraceSingleByChannel(WeaponTraceHit, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);

		if (WeaponTraceHit.bBlockingHit) //Object between barrel and BeamEndPoint!
		{
			OutBeamLocation = WeaponTraceHit.Location;
		}

		
		return true;
	}

	return false;
}



void ASooterCharacter::AimingButtonPressed()
{
	bAiming = true;
}

void ASooterCharacter::AimingButtonReleased()
{
	bAiming = false;
}

void ASooterCharacter::CameraInterpZoom(float DeltaTime)
{
	//set  current camera field of view 
	if (bAiming)
	{
		// interpolate to zoomed fov
		CameraCurrentFov = FMath::FInterpTo(CameraCurrentFov, CameraZoomedFOV, DeltaTime, ZoomInterpSpeed);


	}
	else
	{
		// interpolate to default fov
		CameraCurrentFov = FMath::FInterpTo(CameraCurrentFov, CameraDefaultFOV, DeltaTime, ZoomInterpSpeed);

	}
	GetFollowCamera()->SetFieldOfView(CameraCurrentFov);
}

void ASooterCharacter::SetLookRates()
{
	if (bAiming)
	{
		BaseTurnRate = AimingTurnRate;
		BaseLookUpRate = AimingLookUpRate;
	}
	else
	{
		BaseTurnRate = HipTurnRate;
		BaseLookUpRate = HipLookUpRate;

	}
}

void ASooterCharacter::CalculateCrosshairSpread(float DeltaTime)
{	
	FVector2D WalkSpeedRange{ 0.f, 600.f };
	FVector2D VelocityMultiplierRange{ 0.f, 1.f };
	FVector Velocity{ GetVelocity() };
	Velocity.Z = 0.f;

	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

	CrosshairSpreadMultiplier = 0.5f + CrosshairVelocityFactor;
}

// Called every frame
void ASooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// handle interpolation for zoom when aiming 
	CameraInterpZoom(DeltaTime);
	
	// Change Look sensitive based on aimig
	SetLookRates();
	


}

// Called to bind functionality to input
void ASooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveFoward", this, &ASooterCharacter::MoveFoward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASooterCharacter::MoveRight	);
	PlayerInputComponent->BindAxis("TurnRate", this, &ASooterCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ASooterCharacter::LookUpAtRate);
	PlayerInputComponent->BindAxis("Turn", this, &ASooterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ASooterCharacter::LookUp);


	PlayerInputComponent->BindAction("Jump",IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &ASooterCharacter::FireWeapon);
	
	PlayerInputComponent->BindAction("AimingButton", IE_Pressed, this, &ASooterCharacter::AimingButtonPressed);
	PlayerInputComponent->BindAction("AimingButton", IE_Released, this, &ASooterCharacter::AimingButtonReleased);


}

