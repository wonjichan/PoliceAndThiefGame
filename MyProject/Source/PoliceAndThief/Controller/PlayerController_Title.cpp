#include "PlayerController_Title.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

void APlayerController_Title::BeginPlay()
{
    Super::BeginPlay();

    if (!IsLocalController())
    {
        return;
    }

    if (IsValid(TitleWidget))
    {
        TitleWidgetInstance = CreateWidget<UUserWidget>(this, TitleWidget);
        if (IsValid(TitleWidgetInstance))
        {
            TitleWidgetInstance->AddToViewport();

            FInputModeUIOnly InputModeUIOnly;
            InputModeUIOnly.SetWidgetToFocus(TitleWidgetInstance->GetCachedWidget()); 
            SetInputMode(InputModeUIOnly);

            bShowMouseCursor = true;
        }
    }
}

void APlayerController_Title::JoinServer(const FString& InAddress)
{
    FString DestAddress = InAddress;
    if (DestAddress.IsEmpty())
    {
        DestAddress = TEXT("127.0.0.1");
    }
    ClientTravel(DestAddress, TRAVEL_Absolute);
}
