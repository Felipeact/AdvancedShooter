// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "AmmoType.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_SubMachineGun UMETA(DisplayName = "SubMachineGun"),
	EWT_AssaultRifle UMETA(DisplayName = "AssaultRifle"),
	
	EWT_MAX UMETA(DisplayName = "DefaultMAX")
};

/**
 * 
 */
UCLASS()
class ADVANCEDSHOOTER_API AWeapon : public AItem
{
	GENERATED_BODY()

public:

	AWeapon();

	virtual void Tick(float DeltaTime) override;

protected:


	void StopFalling();

private:

	FTimerHandle ThrowWeaponTimer;

	float ThrowWeaponTime;

	bool bFalling;
	// Ammo Count for this weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapom Properties", meta = (AllowPrivateAccess = "true"))
	int32 Ammo;

	// Maximun Weapon ammo
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapom Properties", meta = (AllowPrivateAccess = "true"))
	int32 MagazineCapacity;

	// The Type of weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapom Properties", meta = (AllowPrivateAccess = "true"))
	EWeaponType WeaponType;

	// The type pf ammo for this weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapom Properties", meta = (AllowPrivateAccess = "true"))
	EAmmoType AmmoType;

	// FName for the reload montage section
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapom Properties", meta = (AllowPrivateAccess = "true"))
	FName ReloadMontageSection;

public:
	// Adds an inpukse to the weapon
	void ThrowWeapon();

	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagazineCapacity() const { return MagazineCapacity; }


	// Called from Character class when firing weapon
	void DecrementAmmo();

	//Geters 
	FORCEINLINE EWeaponType GetWeapontype() const { return WeaponType; }
	FORCEINLINE EAmmoType GetAmmoType() const { return AmmoType; }
	FORCEINLINE FName GetReloadMontageSection() const { return ReloadMontageSection; }

	void ReloadAmmo(int32 Amount);
};
