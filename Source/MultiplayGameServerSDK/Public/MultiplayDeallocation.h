#pragma once

#include "CoreMinimal.h"
#include "MultiplayDeallocation.generated.h"

/**
 * The Multiplay deallocation for the server.
 */
USTRUCT(BlueprintType)
struct MULTIPLAYGAMESERVERSDK_API FMultiplayDeallocation
{
    GENERATED_BODY()

     /**
     * The event ID for the deallocation.
     */
    UPROPERTY(BlueprintReadOnly, Category="Multiplay | Deallocation")
    FString EventId;

    /**
     * The server ID for the deallocation.
     */
    UPROPERTY(BlueprintReadOnly, Category="Multiplay | Deallocation")
    int64 ServerId;

    /**
     * The ID for the deallocation.
     */
    UPROPERTY(BlueprintReadOnly, Category="Multiplay | Deallocation")
    FString AllocationId;
};
