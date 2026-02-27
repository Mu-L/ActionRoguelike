// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RogueMonsterData.generated.h"

class ARogueMonsterCorpse;
class URogueAction;

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API URogueMonsterData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn Info")
	TSubclassOf<AActor> MonsterClass;

	/* Actions/buffs to grant this Monster */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn Info")
	TArray<TSubclassOf<URogueAction>> Actions;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditDefaultsOnly, Category=Corpses)
	TSubclassOf<UAnimInstance> CorpseAnimInstance;

	/*
	 * Allow remapping bones to others in the skeleton to better handle impulses (eg. for corpse ragdolling)
	 */
	UPROPERTY(EditDefaultsOnly, Category=Physics)
	TMap<FName, FName> ImpulseBoneRemapping;
	

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("Monsters", GetFName());
	}
};
