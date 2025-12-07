#include "AISpawn/AISpawn.h"
#include "Engine/World.h"
#include "AI/AIEnemyCharacter.h"
#include "Components/BoxComponent.h"

AAISpawn::AAISpawn()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SetRootComponent(SceneComponent);
	
	SpawningBox = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	SpawningBox->SetupAttachment(SceneComponent);
	SpawningBox->SetBoxExtent(FVector(1400.0f, 1600.0f, 100.0f));
	SpawningBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);	
}

void AAISpawn::SpawnAICharacter(int32 SpawnCount)
{
	if (!IsValid(AICharacterClass) || !IsValid(SpawningBox))
	{
		return;
	}
	
	for (int32 i = 0; i < SpawnCount; i++)
	{
		FVector SpawnLocation = GetRandomSpawnLocation();
		FRotator SpawnRotation = FRotator::ZeroRotator;
		
		AAIEnemyCharacter* SpawnedCharacter = GetWorld()->SpawnActor<AAIEnemyCharacter>(
			AICharacterClass,
			SpawnLocation,
			SpawnRotation);
		
		if (IsValid(SpawnedCharacter))
		{
			SpawnedCharacter->SetReplicates(true);
			SpawnedCharacter->SetReplicateMovement(true);
		}
	}
}

FVector AAISpawn::GetRandomSpawnLocation()
{
	if (!IsValid(SpawningBox))
	{
		return GetActorLocation();
	}
	
	const FVector BoxExtent = SpawningBox->GetScaledBoxExtent();
	const FVector BoxCenter = SpawningBox->GetComponentLocation();
	
	FVector RandomOffset = FVector(
		FMath::RandRange(-BoxExtent.X, BoxExtent.X),
		FMath::RandRange(-BoxExtent.Y, BoxExtent.Y),
		-10.0f
	);
	
	const FQuat BoxRotation = SpawningBox->GetComponentQuat();
	FVector WorldSpawnLocation = BoxCenter + BoxRotation.RotateVector(RandomOffset);
	
	return WorldSpawnLocation;
}