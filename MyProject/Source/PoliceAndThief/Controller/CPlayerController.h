#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CPlayerController.generated.h"

class UUserWidget;
class UGameResultWidget;

UENUM(BlueprintType)
enum class EPlayerRole : uint8
{
    None,
    Police,
    Thief
};

UCLASS()
class POLICEANDTHIEF_API ACPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

    void OnCharacterDead();

    UFUNCTION(Client, Reliable)
    void ClientRPCShowGameResultWidget(int32 InResultCode);

    UFUNCTION(Client, Reliable)
    void ClientRPCReturnToTitle();

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSubclassOf<UGameResultWidget> GameResultUIClass;

    //결과 위젯 재사용하고 싶으면 인스턴스 멤버 추가
    UPROPERTY()
    TObjectPtr<UGameResultWidget> GameResultUIInstance;

public:
    //UI Class 
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSubclassOf<UUserWidget> NoticationTextUIClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UUserWidget> RoleTextUIClass;

    //알림 텍스트 
    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
    FText NotificationText;

    //역할 텍스트 
    UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
    FText RoleText;

    //초기 역할 = None 
    UPROPERTY(Replicated, BlueprintReadOnly)
    EPlayerRole PlayerRole = EPlayerRole::None;

private:
    UPROPERTY()
    TObjectPtr<UUserWidget> RoleTextUIInstance;
};
