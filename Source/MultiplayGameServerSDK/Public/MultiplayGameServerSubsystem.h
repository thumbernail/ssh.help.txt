#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MultiplayAllocation.h"
#include "MultiplayDeallocation.h"
#include "MultiplayErrorResponse.h"
#include "MultiplayPayloadAllocationErrorResponse.h"
#include "MultiplayPayloadTokenResponse.h"
#include "MultiplayGameServerSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAllocateDelegate, FMultiplayAllocation, Allocation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDeallocateDelegate, FMultiplayDeallocation, Deallocation);

DECLARE_DYNAMIC_DELEGATE(FReadyServerSuccessDelegate);
DECLARE_DYNAMIC_DELEGATE_OneParam(FReadyServerFailureDelegate, FMultiplayErrorResponse, ErrorResponse);

DECLARE_DYNAMIC_DELEGATE(FUnreadyServerSuccessDelegate);
DECLARE_DYNAMIC_DELEGATE_OneParam(FUnreadyServerFailureDelegate, FMultiplayErrorResponse, ErrorResponse);

DECLARE_DYNAMIC_DELEGATE_OneParam(FPayloadAllocationSuccessDelegate, FString, Payload);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPayloadAllocationFailureDelegate, FMultiplayPayloadAllocationErrorResponse, ErrorResponse);

DECLARE_DYNAMIC_DELEGATE_OneParam(FPayloadTokenSuccessDelegate, FMultiplayPayloadTokenResponse, TokenResponse);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPayloadTokenFailureDelegate, FMultiplayPayloadTokenResponse, ErrorResponse);

namespace Multiplay
{
	class FCentrifugeClient;
	class FConnectResult;
	class FPublication;

	class OpenAPIGameServerApi;
	class ReadyServerResponse;
	class UnreadyServerResponse;

	class OpenAPIPayloadApi;
	class PayloadAllocationResponse;
	class PayloadTokenResponse;
}

/**
  * @brief Subsystem responsible for communicating with the Multiplay SDK daemon. 
  */
UCLASS()
class MULTIPLAYGAMESERVERSDK_API UMultiplayGameServerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * Subsystem functions, overrides from USubsystem.
	 */
	UMultiplayGameServerSubsystem();
	virtual ~UMultiplayGameServerSubsystem();
	UMultiplayGameServerSubsystem(FVTableHelper& Helper);
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

public:
	// Multiplay Game Server SDK interface.

	/**
	 * @brief Marks the server as ready for players. The client invokes this method when it is ready to start accepting connections. 
	 * @param OnSuccess This delegate will be invoked if the operation completes successfully.
	 * @param OnFailure This delegate will be invoked if the operation is unsuccessful.
	 */
	UFUNCTION(BlueprintCallable, Category="Multiplay | GameServer")
	void ReadyServerForPlayers(FReadyServerSuccessDelegate OnSuccess, FReadyServerFailureDelegate OnFailure);

	/**
	 * @brief Marks the server as not ready for players. The client invokes this method when wants to stop accepting connections.
	 * @param OnSuccess This delegate will be invoked if the operation completes successfully.
	 * @param OnFailure This delegate will be invoked if the operation is unsuccessful.
	 */
	UFUNCTION(BlueprintCallable, Category="Multiplay | GameServer")
	void UnreadyServer(FUnreadyServerSuccessDelegate OnSuccess, FUnreadyServerFailureDelegate OnFailure);

	/**
	 * @brief Establishes a connection to the Multiplay SDK daemon and subscribes to allocation messages.
	 */
	UFUNCTION(BlueprintCallable, Category="Multiplay | GameServer")
	void SubscribeToServerEvents();

	/**
	 * @brief Cleans up connection to the Multiplay SDK daemon and unsubscribe from allocation messages.
	 */
	UFUNCTION(BlueprintCallable, Category="Multiplay | GameServer")
	void UnsubscribeToServerEvents();

	/**
	 * @brief Retrieves the allocation payload.
	 * @param OnSuccess This delegate will be invoked if the operation completes successfully.
	 * @param OnFailure This delegate will be invoked if the operation is unsuccessful.
	 */
	UFUNCTION(BlueprintCallable, Category="Multiplay | GameServer")
	void GetPayloadAllocation(FPayloadAllocationSuccessDelegate OnSuccess, FPayloadAllocationFailureDelegate OnFailure);

	/**
	 * @brief Retrieves a JWT token for payloads.
	 * @param OnSuccess This delegate will be invoked if the operation completes successfully.
	 * @param OnFailure This delegate will be invoked if the operation is unsuccessful.
	 */
	UFUNCTION(BlueprintCallable, Category="Multiplay | GameServer")
	void GetPayloadToken(FPayloadTokenSuccessDelegate OnSuccess, FPayloadTokenFailureDelegate OnFailure);

    /**
     * Delegate that is invoked when this server has been allocated.
     */
	UPROPERTY(BlueprintAssignable, Category="Multiplay | GameServer")
	FAllocateDelegate OnAllocate;

    /**
     * Delegate that is invoked when this server has been deallocated.
     */
	UPROPERTY(BlueprintAssignable, Category="Multiplay | GameServer")
	FDeallocateDelegate OnDeallocate;

private:
	
	/**
	 * @brief Calls when connection messages have been received. 
	 * @param Reply The message body.
	 */
	void OnConnectReply(const Multiplay::FConnectResult& Reply);

	/**
	 * @brief Calls when push messages have been received. Interprets the message as a server event. 
	 * @param Push The message body.
	 */
	void OnPublicationPush(const Multiplay::FPublication& Push);

private:
	/**
	 * @brief Callback invoked when we have received a response to the ReadyServer request.
	 * @param Response The response body.
	 */
	void OnReadyServer(const Multiplay::ReadyServerResponse& Response);

	/**
	 * @brief Callback invoked when we have received a response to the UnreadyServer request.
	 * @param Response The response body.
	 */
	void OnUnreadyServer(const Multiplay::UnreadyServerResponse& Response);

private:
	/**
	 * @brief Callback invoked when we have received a response to the PayloadAllocation request.
	 * @param Response The response body.
	 */
	void OnPayloadAllocation(const Multiplay::PayloadAllocationResponse& Response);

	/**
	 * @brief Callback invoked when we have received a response to the PayloadToken request.
	 * @param Response The response body.
	 */
	void OnPayloadToken(const Multiplay::PayloadTokenResponse& Response);

private:
    /**
     * Holds a reference to the delegate that will be invoked when ReadyServer is successful.
     */
	UPROPERTY()
	FReadyServerSuccessDelegate OnReadyServerSuccess;

    /**
     * Holds a reference to the delegate that will be invoked when ReadyServer is unsuccessful.
     */
	UPROPERTY()
	FReadyServerFailureDelegate OnReadyServerFailure;

    /**
     * Holds a reference to the delegate that will be invoked when UnreadyServer is successful.
     */
	UPROPERTY()
	FUnreadyServerSuccessDelegate OnUnreadyServerSuccess;

    /**
     * Holds a reference to the delegate that will be invoked when UnreadyServer is unsuccessful.
     */
	UPROPERTY()
	FUnreadyServerFailureDelegate OnUnreadyServerFailure;

    /**
     * Holds a reference to the delegate that will be invoked when PayloadAllocation is successful.
     */
	UPROPERTY()
	FPayloadAllocationSuccessDelegate OnPayloadAllocationSuccess;

    /**
     * Holds a reference to the delegate that will be invoked when PayloadAllocation is unsuccessful.
     */
	UPROPERTY()
	FPayloadAllocationFailureDelegate OnPayloadAllocationFailure;

    /**
     * Holds a reference to the delegate that will be invoked when PayloadToken is successful.
     */
	UPROPERTY()
	FPayloadTokenSuccessDelegate OnPayloadTokenSuccess;

    /**
     * Holds a reference to the delegate that will be invoked when PayloadToken is unsuccessful.
     */
	UPROPERTY()
	FPayloadTokenFailureDelegate OnPayloadTokenFailure;

    /**
     * A reference to the client's connection to Centrifuge.
     */
	TUniquePtr<Multiplay::FCentrifugeClient> CentrifugeClient;

    /**
     * A reference to the OpenAPI Game Server API.
     */
	TUniquePtr<Multiplay::OpenAPIGameServerApi> GameServerApi;

    /**
     * A reference to the OpenAPI Payload API.
     */
	TUniquePtr<Multiplay::OpenAPIPayloadApi> PayloadApi;

    /**
     * The unique UUID of the allocation.
     */
	FGuid AllocationId;
};
