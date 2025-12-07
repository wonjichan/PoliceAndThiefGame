#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HandCuff.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UParticleSystemComponent;
class UPrimitiveComponent;     
class APlayerCharacter;

UCLASS()
class POLICEANDTHIEF_API AHandCuff : public AActor
{
	GENERATED_BODY()
	
public:	
	AHandCuff();

protected:
	virtual void BeginPlay() override;

    UFUNCTION()
    void OnOverlapBegin(
        UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

    UFUNCTION()
    void OnEffectFinished(UParticleSystemComponent* ParticleSystem);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UBoxComponent> TriggerBox;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UStaticMeshComponent> Mesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UParticleSystemComponent> PickupEffect;

    // 수갑 하나당 추가되는 목숨 수
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (AllowPrivateAccess = "true"))
    int32 LifeAmount = 1;

};
