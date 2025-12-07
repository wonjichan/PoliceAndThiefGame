#include "Animation/CAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/PlayerCharacter.h"
#include "AI/AIEnemyCharacter.h"

void UCAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!OwnerCharacter)
	{
		OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
		if (OwnerCharacter)
		{
			OwnerCharacterMovementComponent = OwnerCharacter->GetCharacterMovement();
			OwnerPlayerCharacter = Cast<APlayerCharacter>(OwnerCharacter);
		}
	}
	
	if (!IsValid(OwnerCharacter) || !IsValid(OwnerCharacterMovementComponent))
	{
		return;
	}
	
	Velocity = OwnerCharacterMovementComponent->Velocity;
	GroundSpeed = FVector(Velocity.X, Velocity.Y, 0.f).Size();
	bShouldMove = 3.f < GroundSpeed;
	bIsFalling = (OwnerCharacterMovementComponent->IsFalling());
}

void UCAnimInstance::AnimNotify_CheckAttackHit()
{
    ACharacter* OwnerChar = Cast<ACharacter>(TryGetPawnOwner());
    if (!OwnerChar) return;

    UE_LOG(LogTemp, Warning, TEXT("AnimNotify_CheckAttackHit Owner = %s"), *OwnerChar->GetName());

    //플레이어
    if (APlayerCharacter* PC = Cast<APlayerCharacter>(OwnerChar))
    {
        if (!PC->HasAuthority())
        {
            PC->ServerRPCCheckAttackHit();
        }
        return;
    }

    //AI
    if (AAIEnemyCharacter* EC = Cast<AAIEnemyCharacter>(OwnerChar))
    {
        if (EC->HasAuthority())
        {
            EC->CheckAttackHit();   
        }
        return;
    }
}