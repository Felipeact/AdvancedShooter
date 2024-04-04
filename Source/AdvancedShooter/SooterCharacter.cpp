// Fill out your copyright notice in the Description page of Project Settings.


#include "SooterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework//CharacterMovementComponent.h"

// Sets default values
ASooterCharacter::ASooterCharacter() :
	BaseTurnRate(45.f),
	BaseLookUpRate(45.f)
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//create a camera boom (pulls in towards character if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.f; // the camera follows at this distance behind the character 
	CameraBoom->bUsePawnControlRotation = true; // rotate the arm based on the controller 

	//Create a Follow Camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attached Follow Camera to end of CameraBoom
	FollowCamera->bUsePawnControlRotation = false; //Camera does not rotate relative to arm

	// Dont rotate when the controller rotates. Let The controller only affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	//configure character movement 
	GetCharacterMovement()->bOrientRotationToMovement = true; // character moves in tehe direction of input
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);// ... at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;
}

// Called when the game starts or when spawned
void ASooterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
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

// Called every frame
void ASooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);


	PlayerInputComponent->BindAction("Jump",IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

}

