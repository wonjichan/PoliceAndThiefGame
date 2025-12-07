#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AIEnemyCharacter.generated.h"

class UParticleSystem;
class UHealthComponent;

UCLASS()
class POLICEANDTHIEF_API AAIEnemyCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AAIEnemyCharacter();

    virtual void BeginPlay() override;

    void OnHitByAttack();

    void Attack();

    UFUNCTION()
    void ServerRPCCheckAttackHit();

    void CheckAttackHit();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
    float AttackRange = 150.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
    float AttackCooldown = 2.0f;

private:
    UFUNCTION(NetMulticast, Reliable)
    void MulticastRPCDie();

    UFUNCTION(NetMulticast, Reliable)
    void MulticastRPCAttack();

    bool bIsDead = false;

    bool bCanAttack = true;

    UFUNCTION()
    void ResetAttack();

    FTimerHandle AttackCooldownHandle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UHealthComponent> HealthComponent;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UParticleSystem> Particle;

    UFUNCTION()
    void OnDeath();

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
    TObjectPtr<UAnimMontage> AttackMontage;
};
