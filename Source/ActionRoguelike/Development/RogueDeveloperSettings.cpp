// Fill out your copyright notice in the Description page of Project Settings.


#include "Development/RogueDeveloperSettings.h"

#include "Misc/ConfigUtilities.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueDeveloperSettings)

FName URogueDeveloperSettings::GetCategoryName() const
{
	return FApp::GetProjectName();
}

void URogueDeveloperSettings::PostInitProperties()
{
#if WITH_EDITOR
	if (IsTemplate())
	{
		// We want the .ini file to have precedence over the CVar constructor, so we apply the ini to the CVar before following the regular UDeveloperSettingsBackedByCVars flow
		UE::ConfigUtilities::ApplyCVarSettingsFromIni(TEXT("/Script/ActionRoguelike.RogueDeveloperSettings"), *GEditorPerProjectIni, ECVF_SetByProjectSetting);
	}
#endif // WITH_EDITOR

	Super::PostInitProperties();
}