#include "CPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "UI/GameResultWidget.h"
#include "Game/CGameModeBase.h"  

void ACPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (!IsLocalController()) return; 

    //게임 전용 입력
    FInputModeGameOnly GameOnly;
    SetInputMode(GameOnly);
    bShowMouseCursor = false;

    if (IsValid(NoticationTextUIClass))
    {
        UUserWidget* NotificationTextUI = CreateWidget<UUserWidget>(this, NoticationTextUIClass);
        if (IsValid(NotificationTextUI))
        {
            NotificationTextUI->AddToViewport(1);
            NotificationTextUI->SetVisibility(ESlateVisibility::Visible);
        }
    }

    if (IsValid(RoleTextUIClass))
    {
        RoleTextUIInstance = CreateWidget<UUserWidget>(this, RoleTextUIClass);
        if (IsValid(RoleTextUIInstance))
        {
            RoleTextUIInstance->AddToViewport(2);
            RoleTextUIInstance->SetVisibility(ESlateVisibility::Visible);
        }
    }
}

void ACPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ThisClass, NotificationText);
    DOREPLIFETIME(ThisClass, RoleText);
    DOREPLIFETIME(ThisClass, PlayerRole);
}

// ★ Character가 죽었을 때 GameMode에 알려주는 함수
void ACPlayerController::OnCharacterDead()
{
    if (HasAuthority())
    {
        if (ACGameModeBase* GameMode = Cast<ACGameModeBase>(UGameplayStatics::GetGameMode(this)))
        {
            GameMode->OnCharacterDead(this);
        }
    }
}

void ACPlayerController::ClientRPCReturnToTitle_Implementation()
{
    if (IsLocalController())
    {
        UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("Title")), true);
    }
}

void ACPlayerController::ClientRPCShowGameResultWidget_Implementation(int32 InResultCode)
{
    if (!IsLocalController()) return; 

    if (!IsValid(GameResultUIClass)) return; 

    if (!IsValid(GameResultUIInstance))
    {
        GameResultUIInstance = CreateWidget<UGameResultWidget>(this, GameResultUIClass);
        if (!IsValid(GameResultUIInstance)) return; 

        GameResultUIInstance->AddToViewport(3);
    }

    const bool bIsWin = (InResultCode == 1);

    FString ResultString;
    FString TopString = TEXT("작전 결과");
    FString SubResultString; 

    if (PlayerRole == EPlayerRole::Police)
    {
        if (bIsWin)
        {
            ResultString = TEXT("경찰 승리!");
            SubResultString = TEXT("모든 도둑을 제압했습니다.");
        }
        else
        {
            ResultString = TEXT("경찰 패배");
            SubResultString = TEXT("도둑을 놓쳤습니다.");
        }
    }
    else if (PlayerRole == EPlayerRole::Thief)
    {
        if (bIsWin)
        {
            ResultString = TEXT("도둑 승리!");
            SubResultString = TEXT("끝까지 생존했습니다.");
        }
        else
        {
            ResultString = TEXT("도둑 패배");
            SubResultString = TEXT("붙잡히고 말았습니다.");
        }
    }
    else
    {
        ResultString = bIsWin ? TEXT("승리했습니다!") : TEXT("패배했습니다...");
        SubResultString = TEXT("");
    }

    if (IsValid(GameResultUIInstance->TopText))
    {
        GameResultUIInstance->TopText->SetText(FText::FromString(TopString));
    }

    if (IsValid(GameResultUIInstance->ResultText))
    {
        GameResultUIInstance->ResultText->SetText(FText::FromString(ResultString));
    }

    if (IsValid(GameResultUIInstance->SubResultText))
    {
        GameResultUIInstance->SubResultText->SetText(FText::FromString(SubResultString));
    }

    GameResultUIInstance->ResultText->SetText(FText::FromString(ResultString));
    
    FInputModeUIOnly Mode;
    Mode.SetWidgetToFocus(GameResultUIInstance->GetCachedWidget());
    SetInputMode(Mode);

    bShowMouseCursor = true;
}
