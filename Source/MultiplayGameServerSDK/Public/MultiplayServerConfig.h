#pragma once

#include "CoreMinimal.h"
#include "MultiplayServerConfig.generated.h"

/**
 * The server configuration for the current session.
 */
USTRUCT(BlueprintType)
struct MULTIPLAYGAMESERVERSDK_API FMultiplayServerConfig
{
    GENERATED_BODY()

    /**
     * The server ID.
     */
    UPROPERTY(BlueprintReadOnly, Category="Multiplay | ServerConfig")
    int64 ServerId;

    /**
     * The allocation ID.
     */
    UPROPERTY(BlueprintReadOnly, Category="Multiplay | ServerConfig")
    FString AllocationId;

    /**
     * The Server Query Protocol Port.
     */
    UPROPERTY(BlueprintReadOnly, Category="Multiplay | ServerConfig")
    int32 QueryPort;

    /**
     *  The connection port for the session.
     */
    UPROPERTY(BlueprintReadOnly, Category="Multiplay | ServerConfig")
    int32 Port;

    /**
     * The directory logs will be written to.
     */
    UPROPERTY(BlueprintReadOnly, Category="Multiplay | ServerConfig")
    FString ServerLogDirectory;
};
