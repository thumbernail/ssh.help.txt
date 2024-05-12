#pragma once

#include "CoreMinimal.h"
#include "MultiplayAllocation.generated.h"

/**
 * The Multiplay allocation for the server.
 */
USTRUCT(BlueprintType)
struct MULTIPLAYGAMESERVERSDK_API FMultiplayAllocation
{
    GENERATED_BODY()

    /**
     * The event ID for the allocation.
     */
    UPROPERTY(BlueprintReadOnly, Category="Multiplay | Allocation")
    FString EventId;

    /**
     * The server ID for the allocation.
     */
    UPROPERTY(BlueprintReadOnly, Category="Multiplay | Allocation")
    int64 ServerId;

    /**
     * The ID for the allocation.
     */
    UPROPERTY(BlueprintReadOnly, Category="Multiplay | Allocation")
    FString AllocationId;
};
