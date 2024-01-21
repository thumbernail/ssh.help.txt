#include "MultiplayGameServerSDKModule.h"

void FMultiplayGameServerSDKModule::StartupModule()
{
	FModuleManager::Get().LoadModuleChecked("Core");
	FModuleManager::Get().LoadModuleChecked("Http");
	FModuleManager::Get().LoadModuleChecked("Json");
	FModuleManager::Get().LoadModuleChecked("JsonUtilities");
	FModuleManager::Get().LoadModuleChecked("Networking");
	FModuleManager::Get().LoadModuleChecked("Sockets");
	FModuleManager::Get().LoadModuleChecked("WebSockets");
}

void FMultiplayGameServerSDKModule::ShutdownModule()
{
}

IMPLEMENT_MODULE(FMultiplayGameServerSDKModule, MultiplayGameServerSDK)
