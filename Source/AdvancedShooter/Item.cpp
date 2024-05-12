// Fill out your copyright notice in the Description page of Project Settings.


#include "Item.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"
#include "SooterCharacter.h"

// Sets default values
AItem::AItem() :
	ItemName(FString("Default")),
	ItemCount(0),
	ItemRarety(EItemRarity::EIR_Common),
	ItemState(EItemState::EIS_Pickup)
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


// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

