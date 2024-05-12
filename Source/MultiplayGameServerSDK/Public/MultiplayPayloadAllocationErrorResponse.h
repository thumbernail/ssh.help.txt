#pragma once

#include "CoreMinimal.h"
#include "MultiplayPayloadAllocationErrorResponse.generated.h"

USTRUCT(BlueprintType)
struct MULTIPLAYGAMESERVERSDK_API FMultiplayPayloadAllocationErrorResponse
{
	GENERATED_BODY()

	/* the call went through successfully */
	UPROPERTY(BlueprintReadOnly, Category="Multiplay | Errors")
	bool Success = false;

	/* there exists an error */
	UPROPERTY(BlueprintReadOnly, Category="Multiplay | Errors")
	bool Error = false;

	/* code of the error */
	UPROPERTY(BlueprintReadOnly, Category="Multiplay | Errors")
	int32 ErrorCode = 0;

	/* a message describing the error */
	UPROPERTY(BlueprintReadOnly, Category="Multiplay | Errors")
	FString ErrorMessage;
};
