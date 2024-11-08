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
#include "Components/CapsuleComponent.h"
#include "Ammo.h"


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
	MouseAimingTurnRate(0.6f),
	MouseAimingLookUpRate(0.6f),
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
	StartingARAmmo(120),
	//Combat Variables
	CombatState(ECombatState::ECS_Unoccupied),
	bCrouching(false),
	BaseMovementSpeed(650.f),
	CrouchMovementSpeed(300.f),
	StandingCapsuleHalfHeight(88.f),
	CrouchingCapsuleHalfHeight(44.f),
	BaseGroundFriction(2.f),
	CrouchingGroundFriction(100.f),
	bAimingButtonPressed(false),
	bShouldPlayPickupSound(true),
	bShouldPlayEquipSound(true),
	PickupSoundResetTime(0.2f),
	EquipSoundResetTime(0.2f)
	
	


	
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//create a camera boom (pulls in towards character if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 180.f; // the camera follows at this distance behind the character 
	CameraBoom->bUsePawnControlRotation = true; // rotate the arm based on the controller 
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 70.f);

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

	// create handscene component 
	HandSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("HandSceneComp"));

	// Create interpolation components
	WeaponInterpComp = CreateDefaultSubobject<USceneComponent>(TEXT("Weapon Interpolation Component"));
	WeaponInterpComp->SetupAttachment(GetFollowCamera());

	InterpComp1 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 1"));
	InterpComp1->SetupAttachment(GetFollowCamera());

	InterpComp2 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 2"));
	InterpComp2->SetupAttachment(GetFollowCamera());

	InterpComp3 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 3"));
	InterpComp3->SetupAttachment(GetFollowCamera());

	InterpComp4 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 4"));
	InterpComp4->SetupAttachment(GetFollowCamera());

	InterpComp5 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 5"));
	InterpComp5->SetupAttachment(GetFollowCamera());

	InterpComp6 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 6"));
	InterpComp6->SetupAttachment(GetFollowCamera());
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
	GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;


	// Create interp locations struct for each interp location. Add to Array
	InitializeInterpLocations();
	
	
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

	if (CombatState != ECombatState::ECS_Unoccupied) return;

	if (WeaponHasAmmo())
	{
		PLayFireSound();
		SendBullet();
		PlayGunFireMontage();

		//Subtract 1 from the weapon's Ammo
		EquippedWeapon->DecrementAmmo();

		StartFireTimer();
		// Start bullet fire timer for crosshair
		StartCrosshairBulletFire();

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
	bAimingButtonPressed = true;
	if (CombatState != ECombatState::ECS_Reloading)
	{
		Aim();
	}
}

void ASooterCharacter::AimingButtonReleased()
{
	bAimingButtonPressed = false;
	StopAiming();
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

	bFireButtonPressed = true;
	FireWeapon();
}

void ASooterCharacter::FireButtonReleased()
{
	bFireButtonPressed = false;
}

void ASooterCharacter::StartFireTimer()
{
	CombatState = ECombatState::ECS_FireTimerInProgress;
	GetWorldTimerManager().SetTimer(AutoFireTimer, this, &ASooterCharacter::AutoFireReset, AutomaticFireRate);
	
}

void ASooterCharacter::AutoFireReset()
{
	CombatState = ECombatState::ECS_Unoccupied;

	if (WeaponHasAmmo())
	{
		if (bFireButtonPressed)
		{
			FireWeapon();
		}
	}
	else
	{
		ReloadWeapon();
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

void ASooterCharacter::PLayFireSound()
{
	// Play Fire Sound
	if (FireSound)
	{
		UGameplayStatics::PlaySound2D(this, FireSound);
	}
}

void ASooterCharacter::SendBullet()
{
	// Send Bullet 
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
	}
}

void ASooterCharacter::PlayGunFireMontage()
{

	// Play Hip Fire Montagef
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("Start Fire"));
	}
}

void ASooterCharacter::ReloadButtonPressed()
{
	ReloadWeapon();
}

void ASooterCharacter::ReloadWeapon()
{	
	if (CombatState != ECombatState::ECS_Unoccupied) return;

	
	if (EquippedWeapon == nullptr) return;
	

	if (CarryingAmmo() && !EquippedWeapon->ClipIsFull())
	{	

		if (bAiming)
		{
			StopAiming();
		}

		CombatState = ECombatState::ECS_Reloading;
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && ReloadMontage)
		{
			AnimInstance->Montage_Play(ReloadMontage);
			AnimInstance->Montage_JumpToSection(EquippedWeapon->GetReloadMontageSection());
		}
	}
}

void ASooterCharacter::FinishReloading()
{
	
	// update the combat state 
	CombatState = ECombatState::ECS_Unoccupied;

	if (bAimingButtonPressed)
	{
		Aim();
	}
	
	if (EquippedWeapon == nullptr) return;

	const EAmmoType AmmoType = EquippedWeapon->GetAmmoType();

	// update the AmmoMap
	if (AmmoMap.Contains(AmmoType))
	{	
		// Amount of ammo the Character is carrying of the Equipped Weapon Type
		int32 CarriedAmmo = AmmoMap[AmmoType];

		// Space left in the magazine of the equippedWeapon
		const int32 MagEmptySpace = EquippedWeapon->GetMagazineCapacity() - EquippedWeapon->GetAmmo();

		if (MagEmptySpace > CarriedAmmo)
		{
			// Reload the megazine with all the ammo we are carrying
			EquippedWeapon->ReloadAmmo(CarriedAmmo);
			CarriedAmmo = 0;
			AmmoMap.Add(AmmoType, CarriedAmmo);
		}
		else
		{
			// fill the magazine
			EquippedWeapon->ReloadAmmo(MagEmptySpace);
			CarriedAmmo -= MagEmptySpace;
			AmmoMap.Add(AmmoType, CarriedAmmo);
		}
	}
}

bool ASooterCharacter::CarryingAmmo()
{
	if (EquippedWeapon == nullptr) return false;

	EAmmoType AmmoType = EquippedWeapon->GetAmmoType();

	if (AmmoMap.Contains(AmmoType))
	{
		return AmmoMap[AmmoType] > 0;
	}

	return false;
}

void ASooterCharacter::GrabClip()
{
	if (EquippedWeapon == nullptr) return;
	if (HandSceneComponent == nullptr) return;

	// Index for the clip bone on the equipped weapon
	int32 ClipBoneIndex{ EquippedWeapon->GetItemMesh()->GetBoneIndex(EquippedWeapon->GetClipBoneName()) };
	//Store the trasform of the clip
	ClipTransform = EquippedWeapon->GetItemMesh()->GetBoneTransform(ClipBoneIndex);

	FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepRelative, true );
	HandSceneComponent->AttachToComponent(GetMesh(), AttachmentRules, FName(TEXT("Hand_L")));
	HandSceneComponent->SetWorldTransform(ClipTransform);

	EquippedWeapon->SetMovingClip(true);


}

void ASooterCharacter::ReleaseClip()
{
	EquippedWeapon->SetMovingClip(false);

}

void ASooterCharacter::CrouchButtonPressed()
{
	if (!GetCharacterMovement()->IsFalling())
	{
		bCrouching = !bCrouching;
	}
	if (bCrouching)
	{
		GetCharacterMovement()->MaxWalkSpeed = CrouchMovementSpeed;
		GetCharacterMovement()->GroundFriction = CrouchingGroundFriction;
	}
	else 
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
		GetCharacterMovement()->GroundFriction = BaseGroundFriction;
	}
}

void ASooterCharacter::Jump()
{
	if (bCrouching)
	{
		bCrouching = false;
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
	}
	else
	{
		ACharacter::Jump();
	}
}

void ASooterCharacter::InterpCapsuleHalfHeight(float DeltaTime)
{
	float TargetCapsuleHalfHeight{};
	if (bCrouching)
	{
		TargetCapsuleHalfHeight = CrouchingCapsuleHalfHeight;
	}
	else
	{
		TargetCapsuleHalfHeight = StandingCapsuleHalfHeight;
	}
	const float InterpHalfHeight{ FMath::FInterpTo(GetCapsuleComponent()->GetScaledCapsuleHalfHeight(), TargetCapsuleHalfHeight, DeltaTime, 20.f )};


	//negative value is crouching, positive is standing
	const float DeltaCapsuleHalfHeight{ InterpHalfHeight - GetCapsuleComponent()->GetScaledCapsuleHalfHeight() };
	const FVector MeshOffset{ 0.f, 0.f, -DeltaCapsuleHalfHeight };
	GetMesh()->AddLocalOffset(MeshOffset);

	GetCapsuleComponent()->SetCapsuleHalfHeight(InterpHalfHeight);
}

void ASooterCharacter::Aim()
{
	bAiming = true;
	GetCharacterMovement()->MaxWalkSpeed = CrouchMovementSpeed;
}

void ASooterCharacter::StopAiming()
{
	bAiming = false;
	if (!bCrouching)
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
	}
}

void ASooterCharacter::PickupAmmo(AAmmo* Ammo)
{
	//check to see if AmmoMap contains Ammos Ammotype
	if (AmmoMap.Find(Ammo->GetAmmoType()))
	{
		// Get Amount off ammo in our AmmoMap for Ammos type
		int32 AmmoCount{ AmmoMap[Ammo->GetAmmoType()] };
		AmmoCount += Ammo->GetItemCount();

		// Set the Amount of Ammo in the map for this type
		AmmoMap[Ammo->GetAmmoType()] = AmmoCount;
	}

	if (EquippedWeapon->GetAmmoType() == Ammo->GetAmmoType())
	{
		// Check to see if the gun is empty
		if (EquippedWeapon->GetAmmo() == 0)
		{
			ReloadWeapon();
		}
	}

	Ammo->Destroy();
}

void ASooterCharacter::InitializeInterpLocations()
{
	FInterpLocation WeaponLocation{ WeaponInterpComp, 0 };
	InterpLocations.Add(WeaponLocation);

	FInterpLocation InterpLoc1{ InterpComp1, 0 };
	InterpLocations.Add(InterpLoc1);

	FInterpLocation InterpLoc2{ InterpComp2, 0 };
	InterpLocations.Add(InterpLoc2);

	FInterpLocation InterpLoc3{ InterpComp3, 0 };
	InterpLocations.Add(InterpLoc3);

	FInterpLocation InterpLoc4{ InterpComp4, 0 };
	InterpLocations.Add(InterpLoc4);

	FInterpLocation InterpLoc5{ InterpComp5, 0 };
	InterpLocations.Add(InterpLoc5);

	FInterpLocation InterpLoc6{ InterpComp6, 0 };
	InterpLocations.Add(InterpLoc6);
}

int32 ASooterCharacter::GetInterpLocationIndex()
{
	int32 LowestIndex = 1;
	int32 LowestCount = INT_MAX;

	for (int32 i = 1; i < InterpLocations.Num(); i++)
	{
		if (InterpLocations[i].ItemCount < LowestCount)
		{
			LowestIndex = i;
			LowestCount = InterpLocations[i].ItemCount;
		}
	}

	return LowestIndex;
}

void ASooterCharacter::IncrementInterpLocItemCount(int32 Index, int32 Amount)
{
	if (Amount < -1 || Amount > 1) return;

	if (InterpLocations.Num() >= Index)
	{
		InterpLocations[Index].ItemCount += Amount;
	}
}

void ASooterCharacter::StartPickupSoundTimer()
{
	bShouldPlayPickupSound = false;
	GetWorldTimerManager().SetTimer(PickupSoundTimer, this, &ASooterCharacter::ResetPickupSoundTimer, PickupSoundResetTime);
}

void ASooterCharacter::StartEquipSoundTimer()
{
	bShouldPlayEquipSound = false;
	GetWorldTimerManager().SetTimer(EquipSoundTimer, this, &ASooterCharacter::ResetEquipSoundTimer, EquipSoundResetTime);
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

	//Interpolate the capsule half height based on crouching/standing
	InterpCapsuleHalfHeight(DeltaTime);
	


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


	PlayerInputComponent->BindAction("Jump",IE_Pressed, this, &ASooterCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &ASooterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("FireButton", IE_Released, this, &ASooterCharacter::FireButtonReleased);
	
	PlayerInputComponent->BindAction("AimingButton", IE_Pressed, this, &ASooterCharacter::AimingButtonPressed);
	PlayerInputComponent->BindAction("AimingButton", IE_Released, this, &ASooterCharacter::AimingButtonReleased);	
	
	PlayerInputComponent->BindAction("Select", IE_Pressed, this, &ASooterCharacter::SelectButtonPressed);
	PlayerInputComponent->BindAction("Select", IE_Released, this, &ASooterCharacter::SelectButtonReleased);

	PlayerInputComponent->BindAction("ReloadButton", IE_Pressed, this, &ASooterCharacter::ReloadButtonPressed);
	
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASooterCharacter::CrouchButtonPressed);
}

void ASooterCharacter::ResetPickupSoundTimer()
{
	bShouldPlayPickupSound = true;
}

void ASooterCharacter::ResetEquipSoundTimer()
{
	bShouldPlayEquipSound = true;
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

//FVector ASooterCharacter::GetCameraInterpLocation()
//{
//	const FVector CameraWorldLocation{ FollowCamera->GetComponentLocation() };
//	const FVector CameraFoward{ FollowCamera->GetForwardVector() };
//
//	// Desired = CameraWorldLocation + Foward * a + Up * B
//	return CameraWorldLocation + CameraFoward * CameraInterpDistance + FVector(0.f, 0.f, CameraInterpElevation);
//}

void ASooterCharacter::GetPickUpItem(AItem* Item)
{

	Item->PlayEquipSound();

	AWeapon* Weapon = Cast<AWeapon>(Item);

	if (Weapon)
	{
		SwapWeapon(Weapon);
	}

	auto Ammo = Cast<AAmmo>(Item);
	if (Ammo)
	{
		PickupAmmo(Ammo);
	}
}

FInterpLocation ASooterCharacter::GetInterpLocation(int32 Index)
{
	if (Index <= InterpLocations.Num())
	{
		return InterpLocations[Index];
	}

	return FInterpLocation();
}

