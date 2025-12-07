#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AIEnemyController.generated.h"

class UBlackboardComponent;
class UBlackboardData;
class UBehaviorTree;

UCLASS()
class POLICEANDTHIEF_API AAIEnemyController : public AAIController
{
	GENERATED_BODY()
	
public:
	static const float PatrolRadius;
	
	static const FName StartPatrolPositionKey;
	static const FName EndPatrolPositionKey;
	
	AAIEnemyController();

public:
	UFUNCTION()
	void OnCharacterDead();
	
protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void OnPossess(APawn* InPawn) override;
	
	virtual void OnUnPossess() override;
	
	void BeginAI(APawn* InPawn);
	
	void EndAI();

protected:
	//처음 플레이어 인식 거리 
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float DetectRadius = 600.f;

	//AI -> Player 인식 포기 거리 
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float LoseInterestRadius = 750.f;;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (AllowPrivateAccess))
	TObjectPtr<UBlackboardData> BlackboardDataAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (AllowPrivateAccess))
	TObjectPtr<UBehaviorTree> BehaviorTree;
};
