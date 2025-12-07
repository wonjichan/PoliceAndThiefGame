#include "TitleWidget.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Kismet/GameplayStatics.h"
#include "Controller/PlayerController_Title.h"

void UTitleWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (IsValid(StartButton))
    {
        StartButton->OnClicked.AddDynamic(this, &ThisClass::OnStartButtonClicked);
    }
    if (IsValid(ExitButton))
    {
        ExitButton->OnClicked.AddDynamic(this, &ThisClass::OnExitButtonClicked);
    }
}
void UTitleWidget::OnStartButtonClicked()
{
	APlayerController_Title* PlayerController = GetOwningPlayer<APlayerController_Title>();
	if (IsValid(PlayerController))
	{
		FText ServerIP = ServerIPText->GetText();
		PlayerController->JoinServer(ServerIP.ToString());
	}
}

void UTitleWidget::OnExitButtonClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}
