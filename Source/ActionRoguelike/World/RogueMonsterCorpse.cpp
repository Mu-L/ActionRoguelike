// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueMonsterCorpse.h"
#include "ActionRoguelike.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/RogueMonsterData.h"


ARogueMonsterCorpse::ARogueMonsterCorpse()
{
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RagdollMeshComp"));
	MeshComp->SetCollisionProfileName(Collision::Ragdoll_ProfileName);
	RootComponent = MeshComp;

	// @todo: freeze the animation and mesh after it has come to a rest.
}


void ARogueMonsterCorpse::SetCorpseProperties(USkeletalMeshComponent* ReferenceMeshComp, URogueMonsterData* MonsterData)
{
	MeshComp->SetSkeletalMesh(ReferenceMeshComp->GetSkeletalMeshAsset());
	MeshComp->SetAnimInstanceClass(MonsterData->CorpseAnimInstance);

	ImpulseBoneRemapping = MonsterData->ImpulseBoneRemapping;

	FVector LocalOffset = ReferenceMeshComp->GetRelativeLocation();
	FRotator LocalRotationOffset = ReferenceMeshComp->GetRelativeRotation();
	// Add offsets to align the mesh with the expected -90 Z and -90 Yaw often applied to Skeletal Meshes in Blueprint
	MeshComp->AddRelativeLocation(LocalOffset, false, nullptr, ETeleportType::TeleportPhysics);
	MeshComp->AddRelativeRotation(LocalRotationOffset, false, nullptr, ETeleportType::TeleportPhysics);

	// Create empty pose in the new SK we will try to ragdoll	
	FPoseSnapshot& NewPose = MeshComp->GetAnimInstance()->AddPoseSnapshot(PoseSnapshotName);

	// Take snapshot from the original enemy monster that we swap out for this lightweight ragdoll
	ReferenceMeshComp->SnapshotPose(NewPose);

	// Enable rag-doll.
	MeshComp->SetAllBodiesSimulatePhysics(true);
}

bool ARogueMonsterCorpse::AddImpulseAtLocationCustom(FVector Impulse, FVector Location, FName BoneName)
{
	// Check for remapped bones
	if (FName* RemappedBone = ImpulseBoneRemapping.Find(BoneName))
	{
		UE_LOG(LogGame, Log, TEXT("Remapping bone %s to bone %s for Impulse on %s."), *BoneName.ToString(), *RemappedBone->ToString(), *GetName())
		BoneName = *RemappedBone;
	}

	GetMesh()->AddImpulseAtLocation(Impulse, Location, BoneName);

	// Handled
	return true;
}
