#pragma once

#include "CoreMinimal.h"
#include "MultiplayPayloadTokenResponse.generated.h"

USTRUCT(BlueprintType)
struct MULTIPLAYGAMESERVERSDK_API FMultiplayPayloadTokenResponse
{
	GENERATED_BODY()

	/* JWT Token string associated to payload requests */
	UPROPERTY(BlueprintReadOnly, Category="Multiplay | PayloadToken")
	FString Token;

	/* Internal multiplay error occurred retrieving the JWT */
	UPROPERTY(BlueprintReadOnly, Category="Multiplay | PayloadToken")
	FString Error;
};
