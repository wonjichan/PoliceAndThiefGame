#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TitleWidget.generated.h"

class UButton;
class UEditableText;

UCLASS()
class POLICEANDTHIEF_API UTitleWidget : public UUserWidget
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess, BindWidget))
	TObjectPtr<UButton> StartButton;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess, BindWidget))
	TObjectPtr<UButton> ExitButton;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess, BindWidget))
	TObjectPtr<UEditableText> ServerIPText;
	
protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnStartButtonClicked();
	
	UFUNCTION()
	void OnExitButtonClicked();
};
