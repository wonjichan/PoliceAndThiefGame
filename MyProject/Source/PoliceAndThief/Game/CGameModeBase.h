#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CGameModeBase.generated.h"

class AAISpawn;
class ACPlayerController;
class ACPlayerCharacter; 

UCLASS()
class POLICEANDTHIEF_API ACGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;

	virtual void PostLogin(APlayerController* NewPlayer) override;
	
	virtual void Logout(AController* Exiting) override;
	
	void OnCharacterDead(ACPlayerController* InController);

	void OnPoliceCaughtThief(ACPlayerController* Police, ACPlayerController* Thief);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Match")
	int32 MinimumPlayerCountForPlaying = 2;  

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Match")
	int32 WaitingTime = 10;     

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Match")
	int32 EndingTime = 10;
	
	UFUNCTION()
	void AssignRoles();

	bool bRolesAssigned = false;

protected:
	FTimerHandle MainTimerHandle;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<TObjectPtr<ACPlayerController>> AlivePlayerControllers;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<TObjectPtr<ACPlayerController>> DeadPlayerControllers;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<AAISpawn> SpawnBox;

private:
	void OnMainTimerElapsed();
	void NotifyToAllPlayer(const FString& NotificationString);
};
