// Fill out your copyright notice in the Description page of Project Settings.


#include "Item.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"
#include "SooterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

// Sets default values
AItem::AItem() :
	ItemName(FString("Default")),
	ItemCount(0),
	ItemRarety(EItemRarity::EIR_Common),
	ItemState(EItemState::EIS_Pickup),
	// Item Interp Variable
	ItemInterpStartLocation(FVector(0.f)),
	CameraTargetLocation(FVector(0.f)),
	bInterping(false),
	ZCurveTime(0.7f),
	ItemInterpX(0.f),
	ItemInterpY(0.f),
	InterpInitialYawOffset(0.f)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ItemMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ItemMesh"));
	SetRootComponent(ItemMesh);

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(ItemMesh);
	CollisionBox->SetCollisionResponseToChannels(ECollisionResponse::ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	
	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(GetRootComponent());
	
	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	AreaSphere->SetupAttachment(GetRootComponent());

}

// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();

	 // Hide Pickup Widget
	if (PickupWidget) 
	{
		PickupWidget->SetVisibility(false);
	}

	//Sets activerStars array based on item rarety
	SetActiveStars();

	// Setup overlap for area sphere
	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnSphereOverlap );
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AItem::OnSphereEndOverlap);

	//Set item properties based on item states
	SetItemProperties(ItemState);


}

void AItem::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		ASooterCharacter* ShooterCharacter = Cast<ASooterCharacter>(OtherActor);
		if (ShooterCharacter)
		{
			ShooterCharacter->IncrementOverlappedItemCount(1);
		}
	}
}

void AItem::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		ASooterCharacter* ShooterCharacter = Cast<ASooterCharacter>(OtherActor);
		if (ShooterCharacter)
		{
			ShooterCharacter->IncrementOverlappedItemCount(-1);
		}
	}
}

void AItem::SetActiveStars()
{
	//the 0 ekement is not used
	for (int32 i = 0; i <= 5; i++)
	{
		ActiveStarts.Add(false);
	}

	switch (ItemRarety)
	{
	case EItemRarity::EIR_Damaged:
		ActiveStarts[1] = true;
		break;
	case EItemRarity::EIR_Common:
		ActiveStarts[1] = true;
		ActiveStarts[2] = true;
		break;
	case EItemRarity::EIR_UnCommon:
		ActiveStarts[1] = true;
		ActiveStarts[2] = true;
		ActiveStarts[3] = true;
		break;
	case EItemRarity::EIR_Rare:
		ActiveStarts[1] = true;
		ActiveStarts[2] = true;
		ActiveStarts[3] = true;
		ActiveStarts[4] = true;
		break;
	case EItemRarity::EIR_Legendary:
		ActiveStarts[1] = true;
		ActiveStarts[2] = true;
		ActiveStarts[3] = true;
		ActiveStarts[4] = true;
		ActiveStarts[5] = true;
		break;
	//case EItemRarity::EIR_MAX:
	//	break;
	default:
		break;
	}
}

void AItem::SetItemProperties(EItemState State)
{
	switch (State)
	{
	case EItemState::EIS_Pickup:
		// Set Mesh properties
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetEnableGravity(false);
		ItemMesh->SetVisibility(true);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		//Set Area Sphere properties
		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		//Set Collision Box properties
		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

		break;
	case EItemState::EIS_EquipInterping:

		PickupWidget->SetVisibility(false);
		// Set Mesh properties
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetEnableGravity(false);
		ItemMesh->SetVisibility(true);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		//Set Area Sphere properties
		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		//Set Collision Box properties
		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		break;
	case EItemState::EIS_PickedUp:
		break;
	case EItemState::EIS_Equipped:
		PickupWidget->SetVisibility(false);
		// Set Mesh properties
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetEnableGravity(false);
		ItemMesh->SetVisibility(true);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		//Set Area Sphere properties
		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		//Set Collision Box properties
		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		break;
	case EItemState::EIS_Falling:

		// Set Mesh properties
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		ItemMesh->SetSimulatePhysics(true);
		ItemMesh->SetEnableGravity(true);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);

		//Set Area Sphere properties
		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		//Set Collision Box properties
		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		break;
	case EItemState::EIS_MAX:
		break;
	default:
		break;
	}
}

void AItem::FinishInterping()
{
	bInterping = false;
	if ( Character )
	{	
		Character->GetPickUpItem(this);
	}
}

void AItem::ItemInterp(float DeltaTime)
{
	if (!bInterping) return;

	if (Character && ItemZCurve)
	{
		// Elapsed time since we started item interp timer
		const float ElapsedTime = GetWorldTimerManager().GetTimerElapsed(ItemInterpTimer);
		// Get CurveValue corresponding to elapsed time
		const float CurveValue = ItemZCurve->GetFloatValue(ElapsedTime);

		//Get the items initial location whrn the curve started
		FVector ItemLocation = ItemInterpStartLocation;
		//Get Location in front of the camera
		const FVector CameraInterLocation = Character->GetCameraInterpLocation();
		// Vector from item to camera interp location, X and Y are zeroed out
		const FVector ItemToCamera{ FVector(0.f, 0.f, (CameraInterLocation - ItemLocation).Z) };
		// Scale factor to multiply with curve value
		const float DeltaZ = ItemToCamera.Size();

		const FVector CurrentLocation{ GetActorLocation() };
		// Interpoled X value
		const float InterpXValue = FMath::FInterpTo(CurrentLocation.X, CameraInterLocation.X, DeltaTime, 30.0);
		// Interpoled Y value
		const float InterpYValue = FMath::FInterpTo(CurrentLocation.Y, CameraInterLocation.Y, DeltaTime, 30.0);

		// Set X and Y of itemlocation to interped values
		ItemLocation.X = InterpXValue;
		ItemLocation.Y = InterpYValue;


		//Adding curve value to the X Component of the inital location (scaled by delta z)
		ItemLocation.Z += CurveValue * DeltaZ;
		SetActorLocation(ItemLocation, true, nullptr, ETeleportType::TeleportPhysics);

		//Camera Rotation this frame
		const FRotator CameraRotation{Character->GetFollowCamera()->GetComponentRotation()};
		//Camera roation plus initialYawOffset
		FRotator ItemRotation{ 0.f, CameraRotation.Yaw + InterpInitialYawOffset, 0.f };

		SetActorRotation(ItemRotation, ETeleportType::TeleportPhysics);

	}

}


// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	//Handle Item inter when in the EquipInterping State
	ItemInterp(DeltaTime);
}

void AItem::SetItemState(EItemState State)
{
	ItemState = State;
	//Set item properties based on item states
	SetItemProperties(State);
}

void AItem::StartItemCurve(ASooterCharacter* Char)
{
	// Store a handle to the character
	Character = Char;

	// Store initial location of the item
	ItemInterpStartLocation = GetActorLocation();

	bInterping = true;
	SetItemState(EItemState::EIS_EquipInterping);

	GetWorldTimerManager().SetTimer(ItemInterpTimer, this, &AItem::FinishInterping, ZCurveTime);

	// Get Initial Yaw of the camera 
	const float CameraRotationYaw = Character->GetFollowCamera()->GetComponentRotation().Yaw ;
	// Get initial Yaw of the item 
	const float ItemRotationYaw = GetActorRotation().Yaw ;

	// Initial Yaw offset beyween cameara and item
	InterpInitialYawOffset = ItemRotationYaw - CameraRotationYaw;
}

