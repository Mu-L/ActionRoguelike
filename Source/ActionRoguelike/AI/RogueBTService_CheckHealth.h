// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "RogueBTService_CheckHealth.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API URogueBTService_CheckHealth : public UBTService
{
	GENERATED_BODY()

protected:

	UPROPERTY(EditAnywhere, Category = "AI")
	FBlackboardKeySelector LowHealthKey;

	/* Threshold that is considered 'low health' (eg. 0.3 is 30% of maximum hitpoints) */
	UPROPERTY(EditAnywhere, Category = "AI", meta = (ClampMin="0.0", ClampMax="1.0"))
	float LowHealthFraction = 0.3f;

	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
