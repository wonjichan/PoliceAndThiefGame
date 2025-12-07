#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AISpawn.generated.h"

class USceneComponent;
class UBoxComponent;
class AAIEnemyCharacter;

UCLASS()
class POLICEANDTHIEF_API AAISpawn : public AActor
{
	GENERATED_BODY()
	
public:	
	AAISpawn();
	
	UFUNCTION()
	void SpawnAICharacter(int32 SpawnCount);
	
	FVector GetRandomSpawnLocation();

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<USceneComponent> SceneComponent;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UBoxComponent> SpawningBox;
	
	UPROPERTY(EditDefaultsOnly, Category = "Spawn")
	TSubclassOf<AAIEnemyCharacter> AICharacterClass;
};
