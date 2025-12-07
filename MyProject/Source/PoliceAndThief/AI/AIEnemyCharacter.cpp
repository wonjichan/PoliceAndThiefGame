#include "AI/AIEnemyCharacter.h"
#include "AI/AIEnemyController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Component/HealthComponent.h"
#include "Character/PlayerCharacter.h"

AAIEnemyCharacter::AAIEnemyCharacter()
	: bIsDead(false)
{
	PrimaryActorTick.bCanEverTick = false;

	AIControllerClass = AAIEnemyController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	bReplicates = true;
	SetReplicateMovement(true);

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->SetIsReplicated(true);
		MoveComp->bServerAcceptClientAuthoritativePosition = false;
		MoveComp->bCanWalkOffLedgesWhenCrouching = true;
	}

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetComponentTickEnabled(true);
		MeshComp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
		MeshComp->bOwnerNoSee = false;
		MeshComp->bOnlyOwnerSee = false;
		MeshComp->SetTickGroup(ETickingGroup::TG_PrePhysics);
	}

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
}

void AAIEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (!IsPlayerControlled())
	{
		bUseControllerRotationYaw = false;

		if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
		{
			MoveComp->bOrientRotationToMovement = false;
			MoveComp->bUseControllerDesiredRotation = true;
			MoveComp->RotationRate = FRotator(0.f, 480.f, 0.f);
		}
	}

	if (HasAuthority() && IsValid(HealthComponent))
	{
		HealthComponent->OnOutOfHP.AddUObject(this, &AAIEnemyCharacter::OnDeath);
	}
}


void AAIEnemyCharacter::OnHitByAttack()
{
	if (!HasAuthority() || bIsDead)
	{
		return;
	}

	if (IsValid(HealthComponent))
	{
		HealthComponent->ApplyDamage(1);
	}
	else
	{
		OnDeath();
	}
}

void AAIEnemyCharacter::OnDeath()
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;

	MulticastRPCDie();

	if (AAIEnemyController* AICon = Cast<AAIEnemyController>(GetController()))
	{
		AICon->OnCharacterDead();    // 여기서 BT Stop + UnPossess 처리
	}

	Destroy();
}

void AAIEnemyCharacter::Attack()
{
	if (!HasAuthority() || bIsDead) return; 
	if (!bCanAttack) return; 

	bCanAttack = false; 
	GetWorld()->GetTimerManager().SetTimer(
		AttackCooldownHandle,
		this,
		&AAIEnemyCharacter::ResetAttack,
		AttackCooldown,
		false
	);

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
		MoveComp->DisableMovement();

		FTimerHandle TmpHandle;
		GetWorld()->GetTimerManager().SetTimer(
			TmpHandle,
			FTimerDelegate::CreateLambda([this]()
				{
					if (UCharacterMovementComponent* MC = GetCharacterMovement())
					{
						MC->SetMovementMode(MOVE_Walking);
					}
				}),
			0.7f,
			false
		);
	}

	AAIEnemyController* AICon = Cast<AAIEnemyController>(GetController());
	if (AICon)
	{
		if (APawn* Target = Cast<APawn>(AICon->GetFocusActor()))
		{
			FVector Dir = (Target->GetActorLocation() - GetActorLocation());
			Dir.Z = 0.f;
			FRotator NewRot = Dir.Rotation();
			SetActorRotation(NewRot);
		}
	}

	if (!IsValid(AttackMontage) || !IsValid(GetMesh())) return; 

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		if (AnimInstance->Montage_IsPlaying(AttackMontage))
		{
			return;
		}
		//모든 클라에서 보이게 멀티캐스트
		MulticastRPCAttack();

		//서버에서 한번만 재생 
		AnimInstance->Montage_Play(AttackMontage, 1.0f);
	}
}

void AAIEnemyCharacter::ResetAttack()
{
	bCanAttack = true;
}

void AAIEnemyCharacter::MulticastRPCDie_Implementation()
{
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->DisableMovement();
		MoveComp->StopMovementImmediately();
	}

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (IsValid(Particle))
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Particle, GetActorLocation(), GetActorRotation());
	}
}

void AAIEnemyCharacter::MulticastRPCAttack_Implementation()
{
	if (HasAuthority())
	{
		return;
	}

	if (!IsValid(AttackMontage) || !IsValid(GetMesh()))
	{
		return;
	}

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		if (AnimInstance->Montage_IsPlaying(AttackMontage))
		{
			return;
		}

		AnimInstance->Montage_Play(AttackMontage, 1.0f);
	}
}

void AAIEnemyCharacter::ServerRPCCheckAttackHit()
{
	CheckAttackHit();
}

void AAIEnemyCharacter::CheckAttackHit()
{
	if (!HasAuthority())
	{
		return;
	}

	TArray<FHitResult> OutHits;
	FCollisionQueryParams Params(NAME_None, false, this);

	const float FrontOffset = 80.f;
	const float AttackRadius = 70.f;

	const FVector Forward = GetActorForwardVector();
	const FVector Start = GetActorLocation() +
		Forward * (GetCapsuleComponent()->GetScaledCapsuleRadius() + FrontOffset);
	const FVector End = Start + Forward * AttackRange;

	bool bHit = GetWorld()->SweepMultiByObjectType(
		OutHits,
		Start,
		End,
		FQuat::Identity,
		FCollisionObjectQueryParams(ECC_Pawn),
		FCollisionShape::MakeSphere(AttackRadius),
		Params
	);

	if (!bHit) return; 

	TSet<APlayerCharacter*> DamagedPlayers;

	for (const FHitResult& Hit : OutHits)
	{
		if (APlayerCharacter* Player = Cast<APlayerCharacter>(Hit.GetActor()))
		{
			DamagedPlayers.Add(Player);
		}
	}
	for (APlayerCharacter* Player : DamagedPlayers)
	{
		if (IsValid(Player))
		{
			Player->OnHitByAttack(this); //DamagedPlayer -> AI
		}
	}
}