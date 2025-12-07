#include "UI/GameResultWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

void UGameResultWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (!ReturnToTitleButton.Get()->OnClicked.IsAlreadyBound(this, &ThisClass::OnReturnToTitleButtonClicked))
	{
		ReturnToTitleButton.Get()->OnClicked.AddDynamic(this, &ThisClass::OnReturnToTitleButtonClicked);
	}
}

void UGameResultWidget::OnReturnToTitleButtonClicked()
{
	UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("Title")), true);
}
