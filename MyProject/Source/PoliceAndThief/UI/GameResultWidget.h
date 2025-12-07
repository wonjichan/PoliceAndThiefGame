#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameResultWidget.generated.h"

class UTextBlock;
class UButton;

UCLASS()
class POLICEANDTHIEF_API UGameResultWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Meta = (BindWidget))
	TObjectPtr<UTextBlock> ResultText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TopText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SubResultText;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Meta = (BindWidget))
	TObjectPtr<UButton> ReturnToTitleButton;
	
protected:
	virtual void NativeConstruct() override;
	
private:
	UFUNCTION()
	void OnReturnToTitleButtonClicked();
};
