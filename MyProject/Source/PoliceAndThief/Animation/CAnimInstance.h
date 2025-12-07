#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CAnimInstance.generated.h"

class APlayerCharacter;
class APlayer;
class UCharacterMovementComponent;

UCLASS()
class POLICEANDTHIEF_API UCAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
protected:
	UPROPERTY()
	TObjectPtr<ACharacter> OwnerCharacter;
	
	UPROPERTY()
	TObjectPtr<APlayerCharacter> OwnerPlayerCharacter;
	
	UPROPERTY()
	TObjectPtr<UCharacterMovementComponent> OwnerCharacterMovementComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector Velocity;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float GroundSpeed;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	uint8 bShouldMove : 1;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	uint8 bIsFalling : 1;
	
	UFUNCTION()
	void AnimNotify_CheckAttackHit();
};
