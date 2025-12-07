#include "Game/CGameStateBase.h"
#include "Net/UnrealNetwork.h"


void ACGameStateBase::BeginPlay()
{
    Super::BeginPlay();

    // 기본은 대기 상태
    MatchState = EMatchState::Waiting;
    AlivePlayerControllerCount = 0;
    StartCountdown = 0;
    EndCountdown = 0;
}

void ACGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ACGameStateBase, AlivePlayerControllerCount);
    DOREPLIFETIME(ACGameStateBase, MatchState);
    DOREPLIFETIME(ACGameStateBase, StartCountdown);
    DOREPLIFETIME(ACGameStateBase, EndCountdown);
}