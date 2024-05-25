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
#include "Item.h"
#include "Components/WidgetComponent.h"
#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"


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
	CameraZoomedFOV(25.f),
	CameraCurrentFov(0.f),
	ZoomInterpSpeed(20.f),
	// Crosshair spread factor
	CrosshairSpreadMultiplier(0.f),
	CrosshairVelocityFactor(0.f),
	CrosshairInAirFactor(0.f),
	CrosshairAimFactor(0.f),
	CrosshairShootingFactor(0.f),
	// Bullet fire timer variables
	ShootTimeDuration(0.05f),
	bFiringBullet(false),
	//Automatic fire variables
	bFireButtonPressed(false),
	bShouldFire(true),
	AutomaticFireRate(0.1f),
	// item trace variables
	bShouldTraceForItems(false),
	OverlappedItemCount(0),
	// Camera Inter location variables
	CameraInterpDistance(250.f),
	CameraInterpElevation(65.f),
	//Starting Ammo Amount
	Starting9mmAmmo(85),
	StartingARAmmo(120)
	
	


	
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
	GetCharacterMovement()->AirControl = 5.0f;
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


	// Spawn the default weapon and equip it
	EquipWeapon(SpawnDefaultWeapon());


	//
	InitializeAmmoMap();
	
	
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

	if (EquippedWeapon == nullptr) return;

	if (FireSound)
	{
		UGameplayStatics::PlaySound2D(this, FireSound);
	}

	const USkeletalMeshSocket* BarrelSocket = EquippedWeapon->GetItemMesh()->GetSocketByName("BarrelSocket");
	if (BarrelSocket)
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(EquippedWeapon->GetItemMesh());

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
		AnimInstance->Montage_JumpToSection(FName("Start Fire"));
	}

	// Start bullet fire timer for crosshair
	StartCrosshairBulletFire();

	if (EquippedWeapon)
	{	
		//Subtract 1 from the weapon's Ammo
		EquippedWeapon->DecrementAmmo();
	}
}

bool ASooterCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation)
{
	// Check for crosshair trace hit
	FHitResult CrosshairHitResult;
	bool bCrosshairHit = TraceUnderCrosshairs(CrosshairHitResult, OutBeamLocation);

	if (bCrosshairHit)
	{
		// Tentative beam location - still need to trace from gun
		OutBeamLocation = CrosshairHitResult.Location;
	}
	else // no crosshair trace hit
	{
		// OutbeamLocation is the end location for the lane trace
	}

	FHitResult WeaponTraceHit;
	// Perfors second trace from gun barrel

	const FVector WeaponTraceStart{ MuzzleSocketLocation };
	const FVector StartToEnd{ OutBeamLocation - MuzzleSocketLocation };
	const FVector WeaponTraceEnd{ MuzzleSocketLocation + StartToEnd * 1.25f};
	
	GetWorld()->LineTraceSingleByChannel(WeaponTraceHit, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);

	if (WeaponTraceHit.bBlockingHit) //Object between barrel and BeamEndPoint!
	{
		OutBeamLocation = WeaponTraceHit.Location;
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

	// calculate crosshair velocity factor
	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

	// Calculate crosshair in air factor
	if (GetCharacterMovement()->IsFalling()) // is in air?
	{
		//Spread the crosshair slowly while in air
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
	}
	else // character is on ground
	{	
		// Shrink the crosshair rapidly while on the ground
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
	}

	//Calculate crosshair aim factor
	if (bAiming) // Are we aiming>
	{	
		// Shrink Crosshair a small amount very quickly
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.6f, DeltaTime, 30.f);
	}
	else // Not aiming 
	{	
		//Spread crosshair back to normal very quickly
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
	}

	// True 0.05 seconds after firing 
	if (bFiringBullet) 
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.3f, DeltaTime, 60.f);
	}
	else
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 60.f);
	}

	CrosshairSpreadMultiplier = 0.5f + CrosshairVelocityFactor + CrosshairInAirFactor - CrosshairAimFactor + CrosshairShootingFactor;
}

void ASooterCharacter::StartCrosshairBulletFire()
{
	bFiringBullet = true;
	GetWorldTimerManager().SetTimer(CrosshairShootTimer, this, &ASooterCharacter::FinishCrosshairBulletFire, ShootTimeDuration);
}

void ASooterCharacter::FinishCrosshairBulletFire()
{
	bFiringBullet = false;
}

void ASooterCharacter::FireButtonPressed()
{

	if (WeaponHasAmmo())
	{
		bFireButtonPressed = true;
		StartFireTimer();
	}
}

void ASooterCharacter::FireButtonReleased()
{
	bFireButtonPressed = false;
}

void ASooterCharacter::StartFireTimer()
{
	if (bShouldFire)
	{
		FireWeapon();
		bShouldFire = false;
		GetWorldTimerManager().SetTimer(AutoFireTimer, this, &ASooterCharacter::AutoFireReset, AutomaticFireRate);
	}
}

void ASooterCharacter::AutoFireReset()
{
	if (WeaponHasAmmo())
	{
		bShouldFire = true;
		if (bFireButtonPressed)
		{
			StartFireTimer();
		}
	}
}

bool ASooterCharacter::TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation)
{

	// Get Viewport Size
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	//Get Screen space location of crosshair
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	//Get World position and direction of crosshairs
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation, CrosshairWorldPosition, CrosshairWorldDirection);

	if (bScreenToWorld) 
	{
		//Trace from crosshair world location outward
		const FVector Start{ CrosshairWorldPosition };
		const FVector End{ Start + CrosshairWorldDirection * 50'000.f};
		OutHitLocation = End;

		GetWorld()->LineTraceSingleByChannel(OutHitResult, Start, End, ECollisionChannel::ECC_Visibility);


		if (OutHitResult.bBlockingHit)
		{	
			OutHitLocation = OutHitResult.Location;
			return true;
		}
	}

	return false;
}

void ASooterCharacter::TraceForItems()
{
	if (bShouldTraceForItems)
	{

		FHitResult ItemTraceResult;
		FVector HitLocation;
		TraceUnderCrosshairs(ItemTraceResult, HitLocation);
		if (ItemTraceResult.bBlockingHit)
		{
			TraceHitItem = Cast<AItem>(ItemTraceResult.GetActor());
			if (TraceHitItem && TraceHitItem->GetPickupWidget())
			{
				// Shoe Items's Pickup Widget
				TraceHitItem->GetPickupWidget()->SetVisibility(true);

			}

			// We hit an AItem last frame
			if (TraceHitItemLastFrame)
			{
				if (TraceHitItem != TraceHitItemLastFrame)
				{
					// We are hitting a different AItem this frame from last frame or AItem is null
					TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
				}
			}

			// Store a reference to TraceHitItem for next frame 
			TraceHitItemLastFrame = TraceHitItem;
		}
	}

	else if (TraceHitItemLastFrame)
	{
		// No longer overlapping any items, item last fram should not show widget
		TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
	}
}

AWeapon* ASooterCharacter::SpawnDefaultWeapon()
{
	
	// Check the default variable class
	if (DefaultWeaponClass)
	{
		// Spawn the Weapon
		return GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);
		
	}

	return nullptr;
}

void ASooterCharacter::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip)
	{
		

		// Get the Hand Socket
		const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("RightHandSocket"));
		
		if (HandSocket)
		{
			// Attach the Weapon to the right hand socket
			HandSocket->AttachActor(WeaponToEquip, GetMesh());
		}

		EquippedWeapon = WeaponToEquip;
		EquippedWeapon->SetItemState(EItemState::EIS_Equipped);
	}
}

void ASooterCharacter::DropWeapon()
{
	if (EquippedWeapon)
	{
		FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);
		EquippedWeapon->GetItemMesh()->DetachFromComponent(DetachmentTransformRules);

		EquippedWeapon->SetItemState(EItemState::EIS_Falling);
		EquippedWeapon->ThrowWeapon();
	}
}

void ASooterCharacter::SelectButtonPressed()
{
	if (TraceHitItem)
	{

		TraceHitItem->StartItemCurve(this);
		/*AWeapon* TraceHitWeapon = Cast<AWeapon>(TraceHitItem);
		SwapWeapon(TraceHitWeapon);*/
	}
}

void ASooterCharacter::SelectButtonReleased()
{
}

void ASooterCharacter::SwapWeapon(AWeapon* WeaponToSwap)
{
	DropWeapon();
	EquipWeapon(WeaponToSwap);
	TraceHitItem = nullptr;
	TraceHitItemLastFrame = nullptr;

}


void ASooterCharacter::InitializeAmmoMap()
{

	AmmoMap.Add(EAmmoType::EAT_9mm, Starting9mmAmmo);
	AmmoMap.Add(EAmmoType::EAT_AR, StartingARAmmo);
}

bool ASooterCharacter::WeaponHasAmmo()
{
	if (EquippedWeapon == nullptr) return false;
	
	return EquippedWeapon->GetAmmo() > 0;
}

// Called every frame
void ASooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// handle interpolation for zoom when aiming 
	CameraInterpZoom(DeltaTime);
	
	// Change Look sensitive based on aimig
	SetLookRates();

	//Calculate crosshair spread multiplier
	CalculateCrosshairSpread(DeltaTime);
	
	//Check OverlappedItemCount, then trace for items
	TraceForItems();

	
	


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

	PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &ASooterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("FireButton", IE_Released, this, &ASooterCharacter::FireButtonReleased);
	
	PlayerInputComponent->BindAction("AimingButton", IE_Pressed, this, &ASooterCharacter::AimingButtonPressed);
	PlayerInputComponent->BindAction("AimingButton", IE_Released, this, &ASooterCharacter::AimingButtonReleased);	
	
	PlayerInputComponent->BindAction("Select", IE_Pressed, this, &ASooterCharacter::SelectButtonPressed);
	PlayerInputComponent->BindAction("Select", IE_Released, this, &ASooterCharacter::SelectButtonReleased);


}

float ASooterCharacter::GetCrosshairSpreadMultiplier() const
{
	return CrosshairSpreadMultiplier;
}

void ASooterCharacter::IncrementOverlappedItemCount(int8 Amount)
{
	if (OverlappedItemCount + Amount <= 0)
	{
		OverlappedItemCount = 0;
		bShouldTraceForItems = false;
	}
	else
	{
		OverlappedItemCount += Amount;
		bShouldTraceForItems = true;
	}

}

FVector ASooterCharacter::GetCameraInterpLocation()
{
	const FVector CameraWorldLocation{ FollowCamera->GetComponentLocation() };
	const FVector CameraFoward{ FollowCamera->GetForwardVector() };

	// Desired = CameraWorldLocation + Foward * a + Up * B
	return CameraWorldLocation + CameraFoward * CameraInterpDistance + FVector(0.f, 0.f, CameraInterpElevation);
}

void ASooterCharacter::GetPickUpItem(AItem* Item)
{
	AWeapon* Weapon = Cast<AWeapon>(Item);

	if (Weapon)
	{
		SwapWeapon(Weapon);
	}
}

