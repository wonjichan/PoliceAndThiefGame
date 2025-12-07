#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PlayerController_Title.generated.h"

class UUserWidget;

UCLASS()
class POLICEANDTHIEF_API APlayerController_Title : public APlayerController
{
	GENERATED_BODY()
	
private:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> TitleWidget;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UUserWidget> TitleWidgetInstance;
	
public:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void JoinServer(const FString& InAddress);
};
