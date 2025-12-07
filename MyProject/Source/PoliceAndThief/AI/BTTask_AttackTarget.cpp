#include "AI/BTTask_AttackTarget.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AI/AIEnemyController.h"
#include "AI/AIEnemyCharacter.h"

UBTTask_AttackTarget::UBTTask_AttackTarget()
{
    NodeName = TEXT("Attack Target");
}

EBTNodeResult::Type UBTTask_AttackTarget::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    Super::ExecuteTask(OwnerComp, NodeMemory);

    UE_LOG(LogTemp, Warning, TEXT("BTTask_AttackTarget :: ExecuteTask"));   // ¡Ú Ãß°¡

    AAIEnemyController* AICon = Cast<AAIEnemyController>(OwnerComp.GetAIOwner());
    if (!IsValid(AICon))
    {
        UE_LOG(LogTemp, Warning, TEXT("BTTask_AttackTarget : AICon invalid"));
        return EBTNodeResult::Failed;
    }

    AAIEnemyCharacter* AIChar = Cast<AAIEnemyCharacter>(AICon->GetPawn());
    if (!IsValid(AIChar))
    {
        UE_LOG(LogTemp, Warning, TEXT("BTTask_AttackTarget : AIChar invalid"));
        return EBTNodeResult::Failed;
    }

    AIChar->Attack();

    return EBTNodeResult::Succeeded;
}