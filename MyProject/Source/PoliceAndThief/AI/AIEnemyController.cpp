#include "AI/AIEnemyController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Character/PlayerCharacter.h"
#include "Controller/CPlayerController.h"

const float AAIEnemyController::PatrolRadius(500.f);
const FName AAIEnemyController::StartPatrolPositionKey(TEXT("StartPatrolPosition"));
const FName AAIEnemyController::EndPatrolPositionKey(TEXT("EndPatrolPosition"));

AAIEnemyController::AAIEnemyController()
{
    PrimaryActorTick.bCanEverTick = true;

	Blackboard = CreateDefaultSubobject<UBlackboardComponent>(TEXT("Blackboard"));
	BrainComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BrainComponent"));
}

void AAIEnemyController::BeginPlay()
{
	Super::BeginPlay();
	
	APawn* ControlledPawn = GetPawn();
	if (IsValid(ControlledPawn))
	{
		BeginAI(ControlledPawn);
	}
}

void AAIEnemyController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!HasAuthority()) return;

	UBlackboardComponent* BB = Cast<UBlackboardComponent>(Blackboard);

	if (!IsValid(BB)) return; 

	APawn* MyPawn = GetPawn();
	if (!IsValid(MyPawn))
	{
		BB->ClearValue(TEXT("TargetActor"));
		return; 
	}

	APawn* TargetPawn = nullptr; 
	float TargetDistsq = TNumericLimits<float>::Max();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ACPlayerController* PC = Cast<ACPlayerController>(*It))
		{
			// 도둑만 타겟 설정 
			if (PC->PlayerRole != EPlayerRole::Thief)
			{
				continue;
			}

			if (APawn* P = PC->GetPawn())
			{
				const float DistSq = FVector::DistSquared(MyPawn->GetActorLocation(), P->GetActorLocation());

				if (DistSq < TargetDistsq)
				{
					TargetDistsq = DistSq;
					TargetPawn = P;
				}
			}
		}
	}

	if (!IsValid(TargetPawn))
	{
		BB->ClearValue(TEXT("TargetActor"));
		return;
	}

	const float Dist = FMath::Sqrt(TargetDistsq);

	UObject* CurTargetObj = BB->GetValueAsObject(TEXT("TargetActor"));
	AActor* CurTarget = Cast<AActor>(CurTargetObj);

	//아직 타겟이 없는 상태 -> 감지 반경 안에 들어오면 세팅
	if (CurTarget == nullptr)
	{
		if (Dist <= DetectRadius)
		{
			BB->SetValueAsObject(TEXT("TargetActor"), TargetPawn);
		}
	}
	//타겟이 있는 상태 -> 너무 멀어지면 해제
	else
	{
		if (Dist >= LoseInterestRadius)
		{
			BB->ClearValue(TEXT("TargetActor"));
		}
	}
}


void AAIEnemyController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	if (IsValid(InPawn))
	{
		BeginAI(InPawn);
	}
}

void AAIEnemyController::OnUnPossess()
{
	EndAI();
	
	Super::OnUnPossess();
}

void AAIEnemyController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	EndAI();
	
	Super::EndPlay(EndPlayReason);
}

void AAIEnemyController::BeginAI(APawn* InPawn)
{
	if (!IsValid(InPawn) || BehaviorTree == nullptr || BlackboardDataAsset == nullptr)
	{
		return;
	}

	UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(BrainComponent);
	if (IsValid(BTComp) && BTComp->IsRunning())
	{
		return;
	}

	UBlackboardComponent* BB = Cast<UBlackboardComponent>(Blackboard);
	if (IsValid(BB) && UseBlackboard(BlackboardDataAsset, BB))
	{
		BB->SetValueAsVector(StartPatrolPositionKey, InPawn->GetActorLocation());
		RunBehaviorTree(BehaviorTree);
	}
}

void AAIEnemyController::EndAI()
{
	UBehaviorTreeComponent* BehaviorTreeComponent = Cast<UBehaviorTreeComponent>(BrainComponent);
	if (IsValid(BehaviorTreeComponent))
	{
		BehaviorTreeComponent->StopTree();
	}
}

void AAIEnemyController::OnCharacterDead()
{
	// 1) BT 정지
	EndAI(); // 내부에서 StopTree() 이미 함

	// 2) 폰에서 떨어지기
	UnPossess();
}