#include "AI/BTTask_GetEndPatrolPosition.h"

#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AI/AIEnemyCharacter.h"
#include "AI/AIEnemyController.h"

UBTTask_GetEndPatrolPosition::UBTTask_GetEndPatrolPosition()
{
	NodeName = TEXT("GetEndPatrolPosition");
}

EBTNodeResult::Type UBTTask_GetEndPatrolPosition::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type Result = Super::ExecuteTask(OwnerComp, NodeMemory);
	
	if (EBTNodeResult::Failed == Result)
	{
		return Result;
	}
	
	AAIEnemyController* AIEnemyController = Cast<AAIEnemyController>(OwnerComp.GetAIOwner());
	if (IsValid(AIEnemyController))
	{
		AAIEnemyCharacter* AICharacter = Cast<AAIEnemyCharacter>(AIEnemyController->GetPawn());
		
		if (IsValid(AICharacter))
		{
			UNavigationSystemV1* NavigationSystem = UNavigationSystemV1::GetNavigationSystem(AIEnemyController->GetWorld());
			
			if (IsValid(NavigationSystem))
			{
				FVector StartPatrolPosition = OwnerComp.GetBlackboardComponent()->GetValueAsVector(AAIEnemyController::StartPatrolPositionKey);
				FNavLocation EndPatrolPosition;
				if (NavigationSystem->GetRandomPointInNavigableRadius(StartPatrolPosition, AIEnemyController->PatrolRadius,EndPatrolPosition))
				{
					OwnerComp.GetBlackboardComponent()->SetValueAsVector(AAIEnemyController::EndPatrolPositionKey, EndPatrolPosition.Location);
					return Result = EBTNodeResult::Succeeded;
				}
			}
		}
	}
	
	return Result;
}