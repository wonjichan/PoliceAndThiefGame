#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "PlayerCharacter.generated.h"


class UInputAction;
class UInputMappingContext;
class USpringArmComponent;
class UCameraComponent;
class UAnimMontage;
class UHealthComponent;

UCLASS()
class POLICEANDTHIEF_API APlayerCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    APlayerCharacter();

    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    // Component
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Component")
    TObjectPtr<USpringArmComponent> SpringArmComponent;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Component")
    TObjectPtr<UCameraComponent> CameraComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
    TObjectPtr<UHealthComponent> HealthComponent;

    // Input
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputMappingContext> IMC;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> MoveAction;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> LookAction;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> JumpAction;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> AttackAction;

private:
    void MoveInput(const FInputActionValue& Value);
    void LookInput(const FInputActionValue& Value);
    void AttackInput(const FInputActionValue& Value);

public:
    UFUNCTION(Server, Reliable)
    void ServerRPCCheckAttackHit();

    void CheckAttackHit();
    void OnHitByAttack(AActor* Actor);

public:
    //¼ö°© ±â¹Í È£ÃâÇÔ¼ö 
    void AddLifeCoin(int32 Amount);

private:
    void HandleDeath();

    UFUNCTION(Server, Reliable)
    void ServerRPCAttack();

    UFUNCTION(NetMulticast, Unreliable)
    void MulticastRPCAttack();

    UFUNCTION()
    void OnRep_CanAttack();

    void PlayAttackMontage();

    UFUNCTION(NetMulticast, Reliable)
    void MulticastRPCDie(); 

protected:
    UPROPERTY(ReplicatedUsing = OnRep_CanAttack)
    uint8 bCanAttack : 1;

    bool bIsDead = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
    TObjectPtr<UAnimMontage> AttackMontage;

    float AttackMontagePlayTime = 0.f;

};

