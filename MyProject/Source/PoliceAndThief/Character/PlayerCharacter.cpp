#include "PlayerCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"
#include "AI/AIEnemyCharacter.h"
#include "Controller/CPlayerController.h"
#include "Game/CGameModeBase.h"
#include "Game/CGameStateBase.h"
#include "Component/HealthComponent.h"
#include "Kismet/GameplayStatics.h"


APlayerCharacter::APlayerCharacter()
	: bCanAttack(true)
	, AttackMontagePlayTime(0.f)
	, bIsDead(false)
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	GetCharacterMovement()->SetIsReplicated(true);
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComponent->TargetArmLength = 400.0f;
	SpringArmComponent->bUsePawnControlRotation = true;
	SpringArmComponent->SetupAttachment(GetRootComponent());

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);
	CameraComponent->bUsePawnControlRotation = false;

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocallyControlled())
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			if (UEnhancedInputLocalPlayerSubsystem* EILPS =
				ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
			{
				EILPS->AddMappingContext(IMC, 0);
			}
		}
	}

	if (IsValid(AttackMontage))
	{
		AttackMontagePlayTime = AttackMontage->GetPlayLength();
	}

	//서버에서만 죽음 처리 델리게이트 연결
	if (HasAuthority() && IsValid(HealthComponent))
	{
		HealthComponent->OnOutOfHP.AddUObject(this, &APlayerCharacter::HandleDeath);
	}
}


void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::MoveInput);
		EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::LookInput);
		EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &ThisClass::Jump);
		EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ThisClass::StopJumping);
		EIC->BindAction(AttackAction, ETriggerEvent::Started, this, &ThisClass::AttackInput);
	}
}

void APlayerCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APlayerCharacter, bCanAttack);
}

void APlayerCharacter::MoveInput(const FInputActionValue& Value)
{
	if (!IsValid(Controller))
	{
		return;
	}

	const FVector2D MovementVector = Value.Get<FVector2D>();

	const FRotator ControlRotation = Controller->GetControlRotation();
	const FRotator ControlYawRotation(0.0f, ControlRotation.Yaw, 0.0f);

	const FVector ForwardDirection = FRotationMatrix(ControlYawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(ControlYawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.X);
	AddMovementInput(RightDirection, MovementVector.Y);
}

void APlayerCharacter::LookInput(const FInputActionValue& Value)
{
	if (!IsValid(Controller))
	{
		return;
	}

	const FVector2D LookVector = Value.Get<FVector2D>();

	AddControllerYawInput(LookVector.X);
	AddControllerPitchInput(LookVector.Y);
}

void APlayerCharacter::AttackInput(const FInputActionValue& Value)
{
	if (bCanAttack && !GetCharacterMovement()->IsFalling())
	{
		ServerRPCAttack();

		// 로컬 클라에서 먼저 반응
		if (!HasAuthority() && IsLocallyControlled())
		{
			PlayAttackMontage();
		}
	}
}

void APlayerCharacter::ServerRPCCheckAttackHit_Implementation()
{
	CheckAttackHit();
}

void APlayerCharacter::CheckAttackHit()
{
	if (!HasAuthority()) return; 

	TArray<FHitResult> OutHitResults;
	FCollisionQueryParams Params(NAME_None, false, this);

	const float FrontOffset = 80.f;
	const float AttackRange = 150.f;
	const float AttackRadius = 70.f;

	const FVector Forward = GetActorForwardVector();
	const FVector Start = GetActorLocation() +
		Forward * (GetCapsuleComponent()->GetScaledCapsuleRadius() + FrontOffset);
	const FVector End = Start + Forward * AttackRange;

	const bool bIsHitDetected = GetWorld()->SweepMultiByObjectType(
		OutHitResults,
		Start,
		End,
		FQuat::Identity,
		FCollisionObjectQueryParams(ECC_Pawn),
		FCollisionShape::MakeSphere(AttackRadius),
		Params
	);

	if (!bIsHitDetected) return;

	TSet<APlayerCharacter*> DamagedPlayers;

	for (const FHitResult& Hit : OutHitResults)
	{
		if (APlayerCharacter* Player = Cast<APlayerCharacter>(Hit.GetActor()))
		{
			// ★ 컨트롤러 역할 체크
			if (ACPlayerController* PC = Cast<ACPlayerController>(Player->GetController()))
			{
				// 도둑만 맞게
				if (PC->PlayerRole == EPlayerRole::Thief)
				{
					DamagedPlayers.Add(Player);
				}
			}
		}
	}

	for (APlayerCharacter* Player : DamagedPlayers)
	{
		if (IsValid(Player))
		{
			Player->OnHitByAttack(this);
		}
	}
}

void APlayerCharacter::ServerRPCAttack_Implementation()
{
	bCanAttack = false;
	OnRep_CanAttack();

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle,
		FTimerDelegate::CreateLambda([this]()
			{
				bCanAttack = true;
				OnRep_CanAttack();

				if (GetCharacterMovement())
				{
					GetCharacterMovement()->SetMovementMode(MOVE_Walking);
				}
			}),
		AttackMontagePlayTime + 0.5f,
		false
	);

	PlayAttackMontage();
	MulticastRPCAttack();
}

void APlayerCharacter::MulticastRPCDie_Implementation()
{
	// 이동 막기
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->DisableMovement();
		MoveComp->StopMovementImmediately();
	}

	// 캡슐 충돌 끄기
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// 로컬 플레이어라면 입력 막기
	if (IsLocallyControlled())
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			DisableInput(PC);
		}
	}

	// 필요하면 여기서 죽음 애님 몽타주 재생도 가능
	// PlayDeathMontage 같은 거 추가할 수 있음
}
void APlayerCharacter::MulticastRPCAttack_Implementation()
{
	// 서버가 아니고, 내가 조종하는 캐릭터가 아닌 다른 클라들에서만 재생
	if (!HasAuthority() && !IsLocallyControlled())
	{
		PlayAttackMontage();
	}
}


void APlayerCharacter::OnRep_CanAttack()
{
	if (!GetCharacterMovement())
	{
		return;
	}

	if (bCanAttack)
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}
	else
	{
		GetCharacterMovement()->SetMovementMode(MOVE_None);
	}
}

void APlayerCharacter::PlayAttackMontage()
{
	if (!IsValid(AttackMontage) || !IsValid(GetMesh()))
	{
		return;
	}

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->StopAllMontages(0.f);
		AnimInstance->Montage_Play(AttackMontage, 1.0f);
	}
}

void APlayerCharacter::OnHitByAttack(AActor* Actor)
{
	// 서버만 처리 + 이미 죽었으면 무시
	if (!HasAuthority() || bIsDead) return; 

	// 게임 진행 중이 아닐 때는 피격 X
	if (ACGameStateBase* PTGS = Cast<ACGameStateBase>(GetWorld()->GetGameState()))
	{
		if (PTGS->MatchState != EMatchState::Playing)
		{
			return;
		}
	}

	ACPlayerController* MyPC = Cast<ACPlayerController>(GetController());
	if (!IsValid(MyPC)) return; 
	
	if (Actor && Actor->IsA<AAIEnemyCharacter>())
	{
		//경찰 -> AI Enemy 공격 무시 
		if (MyPC->PlayerRole != EPlayerRole::Thief)
		{
			return;
		}
	}

	if (APlayerCharacter* AttackerChar = Cast<APlayerCharacter>(Actor))
	{
		ACPlayerController* AttackerPC = Cast<ACPlayerController>(AttackerChar->GetController());

		if (IsValid(AttackerPC) && IsValid(MyPC))
		{
			// ★ 경찰이 도둑을 공격한 상황 → 즉시 게임 종료
			if (AttackerPC->PlayerRole == EPlayerRole::Police &&
				MyPC->PlayerRole == EPlayerRole::Thief)
			{
				if (ACGameModeBase* GM = Cast<ACGameModeBase>(UGameplayStatics::GetGameMode(this)))
				{
					GM->OnPoliceCaughtThief(AttackerPC, MyPC);
				}
				return;   
			}
		}
	}

	if (IsValid(HealthComponent))
	{
		// HP 감소만 담당
		HealthComponent->ApplyDamage(1);
	}
	else
	{
		// 혹시 모를 예외 상황: HealthComponent 없으면 바로 죽음 처리
		HandleDeath();
	}
}


void APlayerCharacter::HandleDeath()
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;

	if (ACPlayerController* PC = Cast<ACPlayerController>(GetController()))
	{
		PC->OnCharacterDead();
	}

	MulticastRPCDie();

	Destroy();
}

void APlayerCharacter::AddLifeCoin(int32 Amount)
{
	if (!HasAuthority())
	{
		return;
	}

	if (IsValid(HealthComponent))
	{
		if (HealthComponent->GetCurrentHP() < HealthComponent->GetMaxHP())
		{
			HealthComponent->Heal(Amount);
		}
	}
}
