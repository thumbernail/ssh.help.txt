#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "MultiplayGameServerSDKLog.h"

class FMultiplayGameServerSDKModule : public IModuleInterface
{
public:
	// IModuleInterface interface
	void StartupModule();
	void ShutdownModule();
};