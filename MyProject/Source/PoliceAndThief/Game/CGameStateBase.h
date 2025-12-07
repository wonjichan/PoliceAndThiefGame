#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "CGameStateBase.generated.h"

UENUM(BlueprintType)
enum class EMatchState : uint8
{
	None,
	Waiting,
	Playing,
	Ending,
	End
};

UCLASS()
class POLICEANDTHIEF_API ACGameStateBase : public AGameStateBase
{
	GENERATED_BODY()
	
public:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // 살아있는 플레이어 수 
    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
    int32 AlivePlayerControllerCount = 0;

    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
    EMatchState MatchState = EMatchState::Waiting;

    // 게임 시작까지 남은 시간 - Wating 
    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
    int32 StartCountdown = 0;

    // 타이틀로 돌아가기까지 남은 시간 - Ending 
    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
    int32 EndCountdown = 0;
};
