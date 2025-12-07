#include "Component/HealthComponent.h"
#include "Net/UnrealNetwork.h"

UHealthComponent::UHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    MaxHP = 3;
    CurrentHP = MaxHP;

    SetIsReplicatedByDefault(true);
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UHealthComponent, CurrentHP);
}

void UHealthComponent::ApplyDamage(int32 Amount)
{
    if (Amount == 0)
    {
        return;
    }

    const int32 NewHP = FMath::Clamp(CurrentHP - Amount, 0, MaxHP);
    if (NewHP != CurrentHP)
    {
        CurrentHP = NewHP;
        OnCurrentHPChanged.Broadcast(CurrentHP);

        if (CurrentHP <= 0)
        {
            OnOutOfHP.Broadcast();
        }
    }
}

void UHealthComponent::Heal(int32 Amount)
{
    if (Amount <= 0)
    {
        return;
    }

    const int32 NewHP = FMath::Clamp(CurrentHP + Amount, 0, MaxHP);
    if (NewHP != CurrentHP)
    {
        CurrentHP = NewHP;
        OnCurrentHPChanged.Broadcast(CurrentHP);
    }
}

void UHealthComponent::OnRep_CurrentHP()
{
    OnCurrentHPChanged.Broadcast(CurrentHP);
}

void UHealthComponent::AddMaxHP(int32 Amount)
{
    if (Amount == 0)
    {
        return;
    }

    MaxHP = FMath::Max(0, MaxHP + Amount);

    CurrentHP = FMath::Clamp(CurrentHP + Amount, 0, MaxHP);

    OnCurrentHPChanged.Broadcast(CurrentHP);
}