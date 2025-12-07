#include "HandCuff.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Character/PlayerCharacter.h"

AHandCuff::AHandCuff()
{
	PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    SetRootComponent(TriggerBox);

    TriggerBox->SetBoxExtent(FVector(40.f, 40.f, 40.f));
    TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerBox->SetCollisionObjectType(ECC_WorldDynamic);
    TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    TriggerBox->SetGenerateOverlapEvents(true);

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    Mesh->SetupAttachment(TriggerBox);
    Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    PickupEffect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("PickupEffect"));
    PickupEffect->SetupAttachment(TriggerBox);
    PickupEffect->bAutoActivate = false;
}

void AHandCuff::BeginPlay()
{
	Super::BeginPlay();

    TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnOverlapBegin);
}

void AHandCuff::OnOverlapBegin(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);
    if (!IsValid(Player))
    {
        return;
    }

    //서버(ThirdPersonMap) + 클라(Title<->ThirdPersonMap)
    if (IsValid(PickupEffect))
    {
        PickupEffect->Activate(true);
    }

    if (IsValid(Mesh))
    {
        Mesh->SetHiddenInGame(true);
    }

    SetActorEnableCollision(false);

    //서버 로직 
    if (HasAuthority())
    {
        //목숨 증가
        Player->AddLifeCoin(LifeAmount);

        // 파티클 끝나면 삭제, 파티클 없으면 바로 삭제
        if (IsValid(PickupEffect))
        {
            PickupEffect->OnSystemFinished.AddDynamic(
                this, &ThisClass::OnEffectFinished);
        }
        else
        {
            Destroy();
        }
    }
}

void AHandCuff::OnEffectFinished(UParticleSystemComponent* ParticleSystem)
{
    if (HasAuthority())
    {
        Destroy();
    }
}
