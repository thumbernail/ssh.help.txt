#include "MultiplayServerConfigSubsystem.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Engine/GameInstance.h"
#include "Subsystems/SubsystemCollection.h"
#include "MultiplayServerJson.h"
#include "MultiplayGameServerSDKLog.h"

void UMultiplayServerConfigSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

#if PLATFORM_WINDOWS
    FString HomeDrive = FPlatformMisc::GetEnvironmentVariable(TEXT("HOMEDRIVE"));
    FString HomePath = FPlatformMisc::GetEnvironmentVariable(TEXT("HOMEPATH"));
    FString PathToHomeDirectory = FPaths::Combine(HomeDrive, HomePath);
#elif PLATFORM_LINUX
    FString PathToHomeDirectory = FPlatformMisc::GetEnvironmentVariable(TEXT("HOME"));
#else
    FString PathToHomeDirectory = TEXT("");
#endif

    FString PathToServerJson = FPaths::Combine(PathToHomeDirectory, TEXT("server.json"));

    bool bDidReadServerJson = false;
    FString ServerJsonFileContents;
    if (FFileHelper::LoadFileToString(ServerJsonFileContents, *PathToServerJson))
    {
        auto JsonReader = TJsonReaderFactory<>::Create(ServerJsonFileContents);

        TSharedPtr<FJsonValue> ServerJsonValue;
        if (FJsonSerializer::Deserialize(JsonReader, ServerJsonValue) && ServerJsonValue.IsValid())
        {
            Multiplay::FMultiplayServerJson ServerJson;
            if (ServerJson.FromJson(ServerJsonValue))
            {
                UE_LOG(LogMultiplayGameServerSDK, Log, TEXT("Retrieved Server Id: %lld"), ServerJson.ServerId);
                UE_LOG(LogMultiplayGameServerSDK, Log, TEXT("Retrieved Allocation Id: %s"), *ServerJson.AllocationId);
                UE_LOG(LogMultiplayGameServerSDK, Log, TEXT("Retrieved Query Port: %d"), ServerJson.QueryPort);
                UE_LOG(LogMultiplayGameServerSDK, Log, TEXT("Retrieved Port: %d"), ServerJson.Port);
                UE_LOG(LogMultiplayGameServerSDK, Log, TEXT("Retrieved Server Log Directory: %s"), *ServerJson.ServerLogDirectory);

                bDidReadServerJson = true;

                ServerConfig.ServerId = ServerJson.ServerId;
                ServerConfig.AllocationId = ServerJson.AllocationId;
                ServerConfig.QueryPort = ServerJson.QueryPort;
                ServerConfig.Port = ServerJson.Port;
                ServerConfig.ServerLogDirectory = ServerJson.ServerLogDirectory;
            }
        }
    }

    if (!bDidReadServerJson)
    {
        ServerConfig.ServerId = 0;
        ServerConfig.AllocationId = TEXT("");
        ServerConfig.QueryPort = 0;
        ServerConfig.Port = 0;
        ServerConfig.ServerLogDirectory = TEXT("");

#if WITH_EDITOR
        UE_LOG(LogMultiplayGameServerSDK, Warning, TEXT("Failed to read server ID from %s, defaulting to invalid server ID %d."), *PathToServerJson, ServerConfig.ServerId);
#else
        UE_LOG(LogMultiplayGameServerSDK, Error, TEXT("Failed to read server ID from %s."), *PathToServerJson);
#endif

#if WITH_EDITOR
        UE_LOG(LogMultiplayGameServerSDK, Warning, TEXT("Failed to read query port from %s, defaulting to an ephemeral port."), *PathToServerJson);
#else
        UE_LOG(LogMultiplayGameServerSDK, Error, TEXT("Failed to read query port from %s."), *PathToServerJson);
#endif
    }
}

const FMultiplayServerConfig& UMultiplayServerConfigSubsystem::GetServerConfig() const
{
    return ServerConfig;
}
