// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RogueInteractionComponent.generated.h"

class URogueWorldUserWidget;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ACTIONROGUELIKE_API URogueInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	void PrimaryInteract();

protected:

	// Reliable - Will always arrive, eventually. Request will be re-sent unless an acknowledgment was received.
	// Unreliable - Not guaranteed, packet can get lost and won't retry.

	UFUNCTION(Server, Reliable)
	void ServerInteract(AActor* InFocus);

	void FindBestInteractable();

	UPROPERTY(Transient)
	TObjectPtr<AActor> FocusedActor;

	UPROPERTY(EditDefaultsOnly, Category = "Trace")
	float TraceRadius;

	UPROPERTY(EditDefaultsOnly, Category = "Trace")
	TEnumAsByte<ECollisionChannel> TraceChannel;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<URogueWorldUserWidget> DefaultWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<URogueWorldUserWidget> WidgetInst;

public:	

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	URogueInteractionComponent();
	
};