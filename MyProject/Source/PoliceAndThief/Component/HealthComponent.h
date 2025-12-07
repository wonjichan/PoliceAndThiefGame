#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

//HUD - BP HeartHUD에서 실시간 바인딩 
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHPChanged, int32, NewHP);
//죽음 처리용 멀티캐스트 
DECLARE_MULTICAST_DELEGATE(FOnOutOfHP);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class POLICEANDTHIEF_API UHealthComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UHealthComponent();

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health")
    int32 MaxHP = 3;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentHP, Category = "Health")
    int32 CurrentHP = 3;

    UPROPERTY(BlueprintAssignable, Category = "Health")
    FOnHPChanged OnCurrentHPChanged;

    FOnOutOfHP OnOutOfHP;

public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    void ApplyDamage(int32 Amount);
    void Heal(int32 Amount);

public:
    int32 GetCurrentHP() const { return CurrentHP; }
    int32 GetMaxHP() const { return MaxHP; }
    void AddMaxHP(int32 Amount);
protected:
    UFUNCTION()
    void OnRep_CurrentHP();
};
