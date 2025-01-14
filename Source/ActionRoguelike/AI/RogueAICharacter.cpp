// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/RogueAICharacter.h"

#include "ActionRoguelike.h"
#include "AIController.h"
#include "DrawDebugHelpers.h"
#include "BrainComponent.h"
#include "NiagaraComponent.h"
#include "SharedGameplayTags.h"
#include "UI/RogueWorldUserWidget.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ActionSystem/RogueActionComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/AudioComponent.h"
#include "Components/CanvasPanel.h"
#include "Performance/RogueSignificanceComponent.h"
#include "Perception/AISense_Damage.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueAICharacter)


ARogueAICharacter::ARogueAICharacter()
{
	ActionComp = CreateDefaultSubobject<URogueActionComponent>(TEXT("ActionComp"));
	// Set some defaults, ideally we handle this through some data asset instead
	ActionComp->SetDefaultAttributeSet(FRogueMonsterAttributeSet::StaticStruct());

	AttackSoundComp = CreateDefaultSubobject<UAudioComponent>(TEXT("AttackAudioComp"));
	AttackSoundComp->SetupAttachment(RootComponent);
	AttackSoundComp->bAutoManageAttachment = true;
	AttackSoundComp->SetAutoActivate(false);

	// Default set up for the MinionRanged
	AttackFX_Socket = "Muzzle_Front";

	AttackParticleComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("AttackParticleComp"));
	AttackParticleComp->SetupAttachment(GetMesh(), AttackFX_Socket);
	AttackParticleComp->bAutoManageAttachment = true;
	AttackParticleComp->SetAutoActivate(false);

	// Make sure to configure the distance values in Blueprint
	SigManComp = CreateDefaultSubobject<URogueSignificanceComponent>(TEXT("SigManComp"));

	// Ensures we receive a controlled when spawned in the level by our gamemode
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// Enabled on mesh to react to incoming projectiles
	USkeletalMeshComponent* SkelMesh = GetMesh();
	SkelMesh->SetGenerateOverlapEvents(true);
	// Skip performing overlap queries on the Physics Asset after animation (17 queries in case of our MinionRangedBP)
	SkelMesh->bUpdateOverlapsOnAnimationFinalize = false;

	// Skip bones when not visible, may miss anim notifies etc. if animation is skipped so these options must be tested per use case
	SkelMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;

	//TimeToHitParamName = "TimeToHit";
	HitFlash_CustomPrimitiveIndex = 0;
}


void ARogueAICharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// The "simplest" syntax compared to the other convoluted attempts
	FRogueAttribute* FoundAttribute = ActionComp->GetAttribute(SharedGameplayTags::Attribute_Health);
	FoundAttribute->OnAttributeChanged.AddUObject(this, &ThisClass::OnHealthAttributeChanged);
	

	SigManComp->OnSignificanceChanged.AddDynamic(this, &ARogueAICharacter::OnSignificanceChanged);
	
	// Cheap trick to disable until we need it in the health event
	CachedOverlayMaxDistance = GetMesh()->OverlayMaterialMaxDrawDistance;
	GetMesh()->SetOverlayMaterialMaxDrawDistance(1);
}


void ARogueAICharacter::OnHealthAttributeChanged(float NewValue, const FAttributeModification& AttributeModification)
{
	float Delta = AttributeModification.Magnitude;
	AActor* InstigatorActor = AttributeModification.Instigator.Get();
	
	if (Delta < 0.0f)
	{
		// Create once, and skip on instant kill
		if (ActiveHealthBar == nullptr && NewValue > 0.0)
		{
			ActiveHealthBar = CreateWidget<URogueWorldUserWidget>(GetWorld(), HealthBarWidgetClass);
			if (ActiveHealthBar)
			{
				ActiveHealthBar->AttachedActor = this;
				URogueWorldUserWidget::AddToRootCanvasPanel(ActiveHealthBar);
			}
		}

		// Replaces the above "old" method of requiring unique material instances for every mesh element on the player 
		GetMesh()->SetCustomPrimitiveDataFloat(HitFlash_CustomPrimitiveIndex, GetWorld()->TimeSeconds);

		// We can skip rendering this at a distance
		GetMesh()->SetOverlayMaterialMaxDrawDistance(CachedOverlayMaxDistance);

		// After 1.0seconds we should be finished with the hitflash (re-use the handle to reset timer if we get hit again)
		GetWorldTimerManager().SetTimer(OverlayTimerHandle, [this]()
		{
			// Cheap trick to skip rendering this all the time unless we are actively hit flashing
			GetMesh()->SetOverlayMaterialMaxDrawDistance(1);
		}, 1.0f, false);

		// Died
		if (NewValue <= 0.0f)
		{
			// stop BT
			if (HasAuthority())
			{
				AAIController* AIC = GetController<AAIController>();
				AIC->GetBrainComponent()->StopLogic("Killed");

				// Clears active actions, and (de)buffs.
				ActionComp->StopAllActions();
			}
			
			// ragdoll
			GetMesh()->SetAllBodiesSimulatePhysics(true);
			GetMesh()->SetCollisionProfileName(Collision::Ragdoll_ProfileName);

			GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			GetCharacterMovement()->DisableMovement();

			// set lifespan
			SetLifeSpan(10.0f);
		}
		else
		{
			UAISense_Damage::ReportDamageEvent(this, this, InstigatorActor, FMath::Abs(Delta),
				InstigatorActor->GetActorLocation(), GetActorLocation());
		}
	}
}


AActor* ARogueAICharacter::GetTargetActor() const
{
	AAIController* AIC = GetController<AAIController>();
	// Not guaranteed to be possessed (as we may use this in the AnimBP)
	if (AIC)
	{
		return Cast<AActor>(AIC->GetBlackboardComponent()->GetValueAsObject("TargetActor"));
	}
	return nullptr;
}


void ARogueAICharacter::MulticastPlayAttackFX_Implementation()
{
	AttackSoundComp->Play();

	AttackParticleComp->Activate(true);
	
	PlayAnimMontage(AttackMontage);
}

void ARogueAICharacter::OnSignificanceChanged(ESignificanceValue Significance)
{
	// @todo: this may not work perfectly with falling and similar movement modes. (We don't support this on the AI character anyway)
	// NavMesh based walking instead of using world geo

	// @todo: this can crash in Chaos due to not being on the GameThread?
	if (Significance <= ESignificanceValue::Medium)
	{
		//GetCharacterMovement()->SetMovementMode(MOVE_NavWalking);
	}
	else
	{
		//GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}
	
	// Set as 'dormant' if actor is hidden, otherwise we continue ticking the entire character
	const bool bHiddenSignificance = Significance == ESignificanceValue::Hidden;
	SetActorTickEnabled(!bHiddenSignificance);
	GetCharacterMovement()->SetComponentTickEnabled(!bHiddenSignificance);


	EVisibilityBasedAnimTickOption AnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	switch (Significance)
	{
		// Example, force to always tick pose when really nearby. might need the pose even while offscreen
		case ESignificanceValue::Highest:
			AnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
			break;
		case ESignificanceValue::Medium:
			AnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPose;
			break;
		case ESignificanceValue::Lowest:
		case ESignificanceValue::Hidden:
		case ESignificanceValue::Invalid:
			AnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	}
	
	GetMesh()->VisibilityBasedAnimTickOption = AnimTickOption;
}


FGenericTeamId ARogueAICharacter::GetGenericTeamId() const
{
	// Fetch from the AI Controller who has built-in TeamId
	return FGenericTeamId::GetTeamIdentifier(GetController());
}