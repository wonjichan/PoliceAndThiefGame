#include "Game/CGameModeBase.h"
#include "Game/CGameStateBase.h"
#include "Controller/CPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "AISpawn/AISpawn.h"

void ACGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	
	GetWorld()->GetTimerManager().SetTimer(
		MainTimerHandle,
		this,
		&ThisClass::OnMainTimerElapsed,
		1.f,
		true
    );
	
    if (ACGameStateBase* GS = GetGameState<ACGameStateBase>())
    {
        GS->MatchState = EMatchState::Waiting;
        GS->StartCountdown = 0;
        GS->EndCountdown = 0;
    }
}

void ACGameModeBase::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    ACGameStateBase* GS = GetGameState<ACGameStateBase>();
    if (!IsValid(GS))
    {
        return;
    }

    ACPlayerController* PTPC = Cast<ACPlayerController>(NewPlayer);
    if (IsValid(PTPC))
    {
        AlivePlayerControllers.Add(PTPC);
        GS->AlivePlayerControllerCount = AlivePlayerControllers.Num();
    }

    if (GS->MatchState != EMatchState::Waiting)
    {
        NewPlayer->SetLifeSpan(0.1f);
    }
}

void ACGameModeBase::Logout(AController* Exiting)
{
    Super::Logout(Exiting);

    ACGameStateBase* GS = GetGameState<ACGameStateBase>();
    if (!IsValid(GS))
    {
        return;
    }

    ACPlayerController* PTPC = Cast<ACPlayerController>(Exiting);
    if (IsValid(PTPC))
    {
        if (AlivePlayerControllers.Remove(PTPC) > 0)
        {
            GS->AlivePlayerControllerCount = AlivePlayerControllers.Num();
            DeadPlayerControllers.Add(PTPC);
        }
        else
        {
            DeadPlayerControllers.Remove(PTPC);
        }
    }
}

void ACGameModeBase::OnCharacterDead(ACPlayerController* InController)
{
    ACGameStateBase* GS = GetGameState<ACGameStateBase>();
    if (!IsValid(GS) || !IsValid(InController))
    {
        return;
    }

    const int32 Index = AlivePlayerControllers.Find(InController);
    if (Index == INDEX_NONE)
    {
        return;
    }

    InController->ClientRPCShowGameResultWidget(2);  //2 == 패배

    AlivePlayerControllers.RemoveAt(Index);
    DeadPlayerControllers.Add(InController);

    GS->AlivePlayerControllerCount = AlivePlayerControllers.Num();

    int32 ThiefCount = 0;
    TArray<ACPlayerController*> PolicePlayers;

    for (ACPlayerController* PC : AlivePlayerControllers)
    {
        if (!IsValid(PC)) continue;

        if (PC->PlayerRole == EPlayerRole::Thief)
        {
            ++ThiefCount;
        }
        else if (PC->PlayerRole == EPlayerRole::Police)
        {
            PolicePlayers.Add(PC);
        }
    }

    // 3) 도둑이 0명이면 -> 남아있는 경찰 전원 승리
    if (ThiefCount == 0 && PolicePlayers.Num() > 0)
    {
        for (ACPlayerController* PC : PolicePlayers)
        {
            PC->ClientRPCShowGameResultWidget(1);   //1 == 승리
        }

        GS->MatchState = EMatchState::Ending;
        GS->EndCountdown = EndingTime;
    }
}

void ACGameModeBase::OnPoliceCaughtThief(ACPlayerController* Police, ACPlayerController* Thief)
{
    if (!IsValid(Police) || !IsValid(Thief)) return;

    ACGameStateBase* GS = GetGameState<ACGameStateBase>();
    if (!IsValid(GS)) return; 

    //승패 연출
    Police->ClientRPCShowGameResultWidget(1);  //1등 -> 도둑 승 
    Thief->ClientRPCShowGameResultWidget(2);   //2등 -> 도둑 잡힘

    AlivePlayerControllers.Empty();
    AlivePlayerControllers.Add(Police);

    DeadPlayerControllers.Empty();
    DeadPlayerControllers.Add(Thief);

    //엔딩 
    GS->MatchState = EMatchState::Ending;
    GS->EndCountdown = EndingTime;
}

void ACGameModeBase::OnMainTimerElapsed()
{
    ACGameStateBase* GS = GetGameState<ACGameStateBase>();
    if (!IsValid(GS))
    {
        return;
    }

    switch (GS->MatchState)
    {
    case EMatchState::Waiting:
    {
        FString Noti;
        const int32 CurPlayers = AlivePlayerControllers.Num();

        if (CurPlayers < MinimumPlayerCountForPlaying)
        {
            Noti = TEXT("도둑과 경찰이 모일 때까지 대기 중…");
            GS->StartCountdown = 0;
        }
        else
        {
            if (GS->StartCountdown <= 0)
            {
                GS->StartCountdown = WaitingTime;
            }

            Noti = FString::Printf(TEXT("추격전 시작까지 %d초…"), GS->StartCountdown);
            GS->StartCountdown--;
        }
        //2명이상일 떄 + 카운트다운 끝 -> 역할 배정 -> Playing 게임 시작 
        if (CurPlayers >= MinimumPlayerCountForPlaying && GS->StartCountdown <= 0)
        {
            AssignRoles();
            GS->MatchState = EMatchState::Playing;
            Noti = TEXT("추격전 시작!");    
        }

        NotifyToAllPlayer(Noti);
        break;
    }

    case EMatchState::Playing:
    {
        int32 ThiefCount = 0;
        for (ACPlayerController* PC : AlivePlayerControllers)
        {
            if (IsValid(PC) && PC->PlayerRole == EPlayerRole::Thief)
            {
                ++ThiefCount;
            }
        }

        GS->AlivePlayerControllerCount = AlivePlayerControllers.Num();

        FString Noti = FString::Printf(TEXT("도둑 %d명 생존 중"), ThiefCount);
        NotifyToAllPlayer(Noti);

        break;
    }

    case EMatchState::Ending:
    {
        FString Noti = FString::Printf(TEXT("작전 종료까지 %d초…"), GS->EndCountdown);
        NotifyToAllPlayer(Noti);

        GS->EndCountdown--;

        if (GS->EndCountdown < 0)
        {
            for (auto Player : AlivePlayerControllers)
            {
                if (IsValid(Player))
                {
                    Player->ClientRPCReturnToTitle();
                }
            }

            for (auto Player : DeadPlayerControllers)
            {
                if (IsValid(Player))
                {
                    Player->ClientRPCReturnToTitle();
                }
            }

            MainTimerHandle.Invalidate();

            const FName CurrentLevelName(*UGameplayStatics::GetCurrentLevelName(this));
            UGameplayStatics::OpenLevel(this, CurrentLevelName, true, TEXT("listen"));
        }
        break;
    }

    default:
        break;
    }
}

void ACGameModeBase::AssignRoles()
{
    if (bRolesAssigned || AlivePlayerControllers.Num() < 2) return;

    const int32 PoliceIndex = FMath::RandRange(0, AlivePlayerControllers.Num() - 1);

    for (int32 i = 0; i < AlivePlayerControllers.Num(); ++i)
    {
        ACPlayerController* PC = AlivePlayerControllers[i];
        if (!IsValid(PC)) continue;

        if (i == PoliceIndex)
        {
            PC->PlayerRole = EPlayerRole::Police;
            PC->RoleText = FText::FromString(TEXT("경찰"));
        }
        else
        {
            PC->PlayerRole = EPlayerRole::Thief;
            PC->RoleText = FText::FromString(TEXT("도둑"));
        }
    }
    bRolesAssigned = true;
}

void ACGameModeBase::NotifyToAllPlayer(const FString& NotificationString)
{
    const FText Text = FText::FromString(NotificationString);

    for (auto Player : AlivePlayerControllers)
    {
        if (IsValid(Player))
        {
            Player->NotificationText = Text;
        }
    }

    for (auto Player : DeadPlayerControllers)
    {
        if (IsValid(Player))
        {
            Player->NotificationText = Text;
        }
    }
}
