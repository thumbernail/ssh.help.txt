#include "MultiplayGameServerSubsystem.h"
#include "Engine/GameInstance.h"
#include "Subsystems/SubsystemCollection.h"
#include "Centrifuge/MultiplayCentrifugeClient.h"
#include "Centrifuge/MultiplayCentrifugeMessages.h"
#include "MultiplayServerEvents.h"
#include "MultiplayServerConfigSubsystem.h"
#include "OpenAPIGameServerApi.h"
#include "OpenAPIGameServerApiOperations.h"
#include "OpenAPIPayloadApi.h"
#include "OpenAPIPayloadApiOperations.h"
#include "OpenAPIErrorResponseBody.h"
#include "OpenAPIPayloadAllocationErrorResponseBody.h"
#include "OpenAPIPayloadTokenResponseBody.h"
#include "MultiplayGameServerSDKLog.h"

// Necessary to avoid triggering C4150 error for TUniquePtr<FCentrifugeClient> because FCentrifugeClient is forward declared.
// See documentation in TDefaultDelete<T>::operator() for an explanation.
UMultiplayGameServerSubsystem::UMultiplayGameServerSubsystem() = default;
UMultiplayGameServerSubsystem::~UMultiplayGameServerSubsystem() = default;
UMultiplayGameServerSubsystem::UMultiplayGameServerSubsystem(FVTableHelper& Helper) : UGameInstanceSubsystem(Helper) {}

void UMultiplayGameServerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

    // This subsystem is dependent on the server.json file having been parsed so that the serverID value can be retrieved.
    Collection.InitializeDependency(UMultiplayServerConfigSubsystem::StaticClass());

	FString SdkDaemonIp = TEXT("localhost");
	uint16 SdkDaemonPort = 8086;
	FString SdkDaemonUrl = FString::Printf(TEXT("http://%s:%u"), *SdkDaemonIp, SdkDaemonPort);
	FString SdkDaemonCentrifugeEndpoint = FString::Printf(TEXT("ws://%s:%u/v1/connection/websocket"), *SdkDaemonIp, SdkDaemonPort);

	GameServerApi = MakeUnique<Multiplay::OpenAPIGameServerApi>();
	GameServerApi->SetURL(SdkDaemonUrl);

	PayloadApi = MakeUnique<Multiplay::OpenAPIPayloadApi>();
	PayloadApi->SetURL(SdkDaemonUrl);

	CentrifugeClient = MakeUnique<Multiplay::FCentrifugeClient>(SdkDaemonCentrifugeEndpoint);
	CentrifugeClient->OnConnectReply().AddUObject(this, &UMultiplayGameServerSubsystem::OnConnectReply);
	CentrifugeClient->OnPublicationPush().AddUObject(this, &UMultiplayGameServerSubsystem::OnPublicationPush);
}

void UMultiplayGameServerSubsystem::Deinitialize()
{
	CentrifugeClient->Disconnect();
	CentrifugeClient->OnConnectReply().RemoveAll(this);
	CentrifugeClient->OnPublicationPush().RemoveAll(this);

	Super::Deinitialize();
}

bool UMultiplayGameServerSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	// TODO: Provide a way to conditionally disable this subsystem outside of Multiplay.
	return true;
}

void UMultiplayGameServerSubsystem::OnConnectReply(const Multiplay::FConnectResult& Result)
{
	UE_LOG(LogMultiplayGameServerSDK, Verbose, TEXT("UMultiplayGameServerSubsystem::OnConnectReply()"));

	UMultiplayServerConfigSubsystem* Subsystem = GetGameInstance()->GetSubsystem<UMultiplayServerConfigSubsystem>();
	const FMultiplayServerConfig& ServerConfig = Subsystem->GetServerConfig();
	int64 ServerId = ServerConfig.ServerId;

	Multiplay::FSubscribeRequest request = Multiplay::FSubscribeRequest();
	request.Channel = FString::Printf(TEXT("server#%lld"), ServerId);
	CentrifugeClient->Subscribe(request);
}

void UMultiplayGameServerSubsystem::OnPublicationPush(const Multiplay::FPublication& Push)
{
	UE_LOG(LogMultiplayGameServerSDK, Verbose, TEXT("UMultiplayGameServerSubsystem::OnPublicationPush()"));

	Multiplay::FMultiplayServerAllocateEvent AllocateEvent;
	Multiplay::FMultiplayServerDeallocateEvent DeallocateEvent;

	if (AllocateEvent.FromJson(Push.Data))
	{
		UE_LOG(LogMultiplayGameServerSDK, Log, TEXT("Successfully parsed FMultiplayServerAllocateEvent"));

		AllocationId = AllocateEvent.AllocationId;

		FMultiplayAllocation MultiplayAllocation;
		MultiplayAllocation.EventId = AllocateEvent.EventId.ToString();
		MultiplayAllocation.ServerId = AllocateEvent.ServerId;
		MultiplayAllocation.AllocationId = AllocateEvent.AllocationId.ToString();

		OnAllocate.Broadcast(MultiplayAllocation);
	}
	else if (DeallocateEvent.FromJson(Push.Data))
	{
		UE_LOG(LogMultiplayGameServerSDK, Log, TEXT("Successfully parsed FMultiplayServerDeallocateEvent"));

		AllocationId.Invalidate();

		FMultiplayDeallocation MultiplayDeallocation;
		MultiplayDeallocation.EventId = DeallocateEvent.EventId.ToString();
		MultiplayDeallocation.ServerId = DeallocateEvent.ServerId;
		MultiplayDeallocation.AllocationId = DeallocateEvent.AllocationId.ToString();

		OnDeallocate.Broadcast(MultiplayDeallocation);
	}
	else
	{
		UE_LOG(LogMultiplayGameServerSDK, Warning, TEXT("Failed to parse PUSH message into an event!"));
	}
}

void UMultiplayGameServerSubsystem::ReadyServerForPlayers(FReadyServerSuccessDelegate OnSuccess, FReadyServerFailureDelegate OnFailure)
{
	UE_LOG(LogMultiplayGameServerSDK, Verbose, TEXT("UMultiplayGameServerSubsystem::ReadyServerForPlayers()"));

	OnReadyServerSuccess = OnSuccess;
	OnReadyServerFailure = OnFailure;

	if (AllocationId.IsValid()) 
	{
        UMultiplayServerConfigSubsystem* Subsystem = GetGameInstance()->GetSubsystem<UMultiplayServerConfigSubsystem>();
        const FMultiplayServerConfig& ServerConfig = Subsystem->GetServerConfig();
        int64 ServerId = ServerConfig.ServerId;

		Multiplay::ReadyServerRequest Request;
		Request.ServerId = ServerId;
		Request.AllocationId = AllocationId;

		Multiplay::FReadyServerDelegate Delegate =
			Multiplay::FReadyServerDelegate::CreateUObject(this, &UMultiplayGameServerSubsystem::OnReadyServer);

		GameServerApi->ReadyServer(Request, Delegate);
	}
	else
	{
		FMultiplayErrorResponse InvalidAllocationResponse;
		InvalidAllocationResponse.Title = TEXT("Invalid Allocation ID");
		InvalidAllocationResponse.Detail = TEXT("Attempted invoke ReadyServerForPlayers() with an invalid allocation ID.");
		InvalidAllocationResponse.Status = 400;
		OnReadyServerFailure.Execute(InvalidAllocationResponse);
	}
}

void UMultiplayGameServerSubsystem::UnreadyServer(FUnreadyServerSuccessDelegate OnSuccess, FUnreadyServerFailureDelegate OnFailure)
{
	UE_LOG(LogMultiplayGameServerSDK, Verbose, TEXT("UMultiplayGameServerSubsystem::UnreadyServer()"));

	OnUnreadyServerSuccess = OnSuccess;
	OnUnreadyServerFailure = OnFailure;

    UMultiplayServerConfigSubsystem* Subsystem = GetGameInstance()->GetSubsystem<UMultiplayServerConfigSubsystem>();
    const FMultiplayServerConfig& ServerConfig = Subsystem->GetServerConfig();
    int64 ServerId = ServerConfig.ServerId;

	Multiplay::UnreadyServerRequest Request;
	Request.ServerId = ServerId;

	Multiplay::FUnreadyServerDelegate Delegate =
		Multiplay::FUnreadyServerDelegate::CreateUObject(this, &UMultiplayGameServerSubsystem::OnUnreadyServer);

	GameServerApi->UnreadyServer(Request, Delegate);
}

void UMultiplayGameServerSubsystem::SubscribeToServerEvents()
{
	UE_LOG(LogMultiplayGameServerSDK, Verbose, TEXT("UMultiplayGameServerSubsystem::SubscribeToServerEvents()"));

	Multiplay::FConnectRequest Request;
	CentrifugeClient->Connect(Request);
}

void UMultiplayGameServerSubsystem::UnsubscribeToServerEvents()
{
	UE_LOG(LogMultiplayGameServerSDK, Verbose, TEXT("UMultiplayGameServerSubsystem::UnsubscribeToServerEvents()"));

	CentrifugeClient->Disconnect();
}

void UMultiplayGameServerSubsystem::GetPayloadAllocation(FPayloadAllocationSuccessDelegate OnSuccess, FPayloadAllocationFailureDelegate OnFailure)
{
	UE_LOG(LogMultiplayGameServerSDK, Verbose, TEXT("UMultiplayGameServerSubsystem::GetPayloadAllocation()"));

	OnPayloadAllocationSuccess = OnSuccess;
	OnPayloadAllocationFailure = OnFailure;

	Multiplay::PayloadAllocationRequest Request;
	Request.AllocationId = AllocationId;

	Multiplay::FPayloadAllocationDelegate Delegate =
		Multiplay::FPayloadAllocationDelegate::CreateUObject(this, &UMultiplayGameServerSubsystem::OnPayloadAllocation);

	PayloadApi->PayloadAllocation(Request, Delegate);
}

void UMultiplayGameServerSubsystem::GetPayloadToken(FPayloadTokenSuccessDelegate OnSuccess, FPayloadTokenFailureDelegate OnFailure)
{
	UE_LOG(LogMultiplayGameServerSDK, Verbose, TEXT("UMultiplayGameServerSubsystem::GetPayloadToken()"));

	OnPayloadTokenSuccess = OnSuccess;
	OnPayloadTokenFailure = OnFailure;

	Multiplay::PayloadTokenRequest Request;

	Multiplay::FPayloadTokenDelegate Delegate =
		Multiplay::FPayloadTokenDelegate::CreateUObject(this, &UMultiplayGameServerSubsystem::OnPayloadToken);

	PayloadApi->PayloadToken(Request, Delegate);
}

void UMultiplayGameServerSubsystem::OnReadyServer(const Multiplay::ReadyServerResponse& Response)
{
	if (Response.IsSuccessful())
	{
		UE_LOG(LogMultiplayGameServerSDK, Log, TEXT("ServerReady() was successful"));
		OnReadyServerSuccess.ExecuteIfBound();
	}
	else
	{
		const FHttpResponsePtr& HttpResponse = Response.GetHttpResponse();
		int32 ResponseCode = HttpResponse->GetResponseCode();
		FString ResponseBody = HttpResponse->GetContentAsString();
		TSharedPtr<FJsonValue> JsonParsed;
		TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(ResponseBody);
		
		FMultiplayErrorResponse MultiplayErrorResponseBodyStruct = {};

		if (FJsonSerializer::Deserialize(JsonReader, JsonParsed) && JsonParsed.IsValid())
		{
			TUniquePtr<Multiplay::OpenAPIErrorResponseBody> ErrorResponseBody = MakeUnique<Multiplay::OpenAPIErrorResponseBody>();
			if (ErrorResponseBody->FromJson(JsonParsed)) 
			{
				MultiplayErrorResponseBodyStruct.Status = ErrorResponseBody->Status;
				MultiplayErrorResponseBodyStruct.Detail = ErrorResponseBody->Detail;
				MultiplayErrorResponseBodyStruct.Title = ErrorResponseBody->Title;
			}
			else 
			{
				MultiplayErrorResponseBodyStruct.Status = 500;
				MultiplayErrorResponseBodyStruct.Detail = TEXT("Failed to parse ReadyServer JSON response body on failure");
				MultiplayErrorResponseBodyStruct.Title = TEXT("Parsing failed error");
			}
		}
		else
		{
			MultiplayErrorResponseBodyStruct.Status = 500;
			MultiplayErrorResponseBodyStruct.Detail = TEXT("Failed to deserialize ReadyServer JSON response body on failure");
			MultiplayErrorResponseBodyStruct.Title = TEXT("Parsing failed error");
		}

		UE_LOG(LogMultiplayGameServerSDK, Error, TEXT("ServerReady() was unsuccessful, response status code is '%d' and response body is '%s'"), ResponseCode, *ResponseBody);

		OnReadyServerFailure.ExecuteIfBound(MultiplayErrorResponseBodyStruct);
	}
}

void UMultiplayGameServerSubsystem::OnUnreadyServer(const Multiplay::UnreadyServerResponse& Response)
{
	if (Response.IsSuccessful())
	{
		UE_LOG(LogMultiplayGameServerSDK, Log, TEXT("ServerUnready() was successful"));

		OnUnreadyServerSuccess.ExecuteIfBound();
	}
	else
	{
		const FHttpResponsePtr& HttpResponse = Response.GetHttpResponse();
		int32 ResponseCode = HttpResponse->GetResponseCode();
		FString ResponseBody = HttpResponse->GetContentAsString();
		TSharedPtr<FJsonValue> JsonParsed;
		TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(ResponseBody);

		FMultiplayErrorResponse MultiplayErrorResponseBodyStruct = {};

		if (FJsonSerializer::Deserialize(JsonReader, JsonParsed) && JsonParsed.IsValid())
		{
			TUniquePtr<Multiplay::OpenAPIErrorResponseBody> ErrorResponseBody = MakeUnique<Multiplay::OpenAPIErrorResponseBody>();
			if (ErrorResponseBody->FromJson(JsonParsed))
			{
				MultiplayErrorResponseBodyStruct.Status = ErrorResponseBody->Status;
				MultiplayErrorResponseBodyStruct.Detail = ErrorResponseBody->Detail;
				MultiplayErrorResponseBodyStruct.Title = ErrorResponseBody->Title;
			}
			else
			{
				MultiplayErrorResponseBodyStruct.Status = 500;
				MultiplayErrorResponseBodyStruct.Detail = TEXT("Failed to parse ReadyServer JSON response body on failure");
				MultiplayErrorResponseBodyStruct.Title = TEXT("Parsing failed error");
			}
		}
		else
		{
			MultiplayErrorResponseBodyStruct.Status = 500;
			MultiplayErrorResponseBodyStruct.Detail = TEXT("Failed to deserialize ReadyServer JSON response body on failure");
			MultiplayErrorResponseBodyStruct.Title = TEXT("Parsing failed error");
		}

		UE_LOG(LogMultiplayGameServerSDK, Error, TEXT("OnUnreadyServer() was unsuccessful, response status code is '%d' and response body is '%s'"), ResponseCode, *ResponseBody);

		OnUnreadyServerFailure.ExecuteIfBound(MultiplayErrorResponseBodyStruct);
	}
}

void UMultiplayGameServerSubsystem::OnPayloadAllocation(const Multiplay::PayloadAllocationResponse& Response)
{
	if (Response.IsSuccessful())
	{
		UE_LOG(LogMultiplayGameServerSDK, Log, TEXT("PayloadAllocation() was successful"));
		const FHttpResponsePtr& HttpResponse = Response.GetHttpResponse();
		FString ResponseBody = HttpResponse->GetContentAsString();
		OnPayloadAllocationSuccess.ExecuteIfBound(ResponseBody);
	}
	else
	{
		const FHttpResponsePtr& HttpResponse = Response.GetHttpResponse();
		int32 ResponseCode = HttpResponse->GetResponseCode();
		FString ResponseBody = HttpResponse->GetContentAsString();
		TSharedPtr<FJsonValue> JsonParsed;
		TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(ResponseBody);
		
		FMultiplayPayloadAllocationErrorResponse MultiplayErrorResponseBodyStruct = {};
		
		if (FJsonSerializer::Deserialize(JsonReader, JsonParsed) && JsonParsed.IsValid())
		{
			TUniquePtr<Multiplay::OpenAPIPayloadAllocationErrorResponseBody> ErrorResponseBody = MakeUnique<Multiplay::OpenAPIPayloadAllocationErrorResponseBody>();
			if (ErrorResponseBody->FromJson(JsonParsed))
			{
				MultiplayErrorResponseBodyStruct.Error = ErrorResponseBody->Error;
				MultiplayErrorResponseBodyStruct.ErrorCode = ErrorResponseBody->ErrorCode;
				MultiplayErrorResponseBodyStruct.ErrorMessage = ErrorResponseBody->ErrorMessage;
				MultiplayErrorResponseBodyStruct.Success = ErrorResponseBody->Success;
			}
			else
			{
				MultiplayErrorResponseBodyStruct.Error = true;
				MultiplayErrorResponseBodyStruct.ErrorCode = 500;
				MultiplayErrorResponseBodyStruct.ErrorMessage = TEXT("Failed to parse JSON response body on failure");
				MultiplayErrorResponseBodyStruct.Success = false;
			}
		}
		else 
		{
			MultiplayErrorResponseBodyStruct.Error = true;
			MultiplayErrorResponseBodyStruct.ErrorCode = 500;
			MultiplayErrorResponseBodyStruct.ErrorMessage = TEXT("Failed to deserialize JSON response body on failure");
			MultiplayErrorResponseBodyStruct.Success = false;
		}

		UE_LOG(LogMultiplayGameServerSDK, Error, TEXT("OnPayloadAllocation() was unsuccessful, response status code is '%d' and response body is '%s'"), ResponseCode, *ResponseBody);

		OnPayloadAllocationFailure.ExecuteIfBound(MultiplayErrorResponseBodyStruct);
	}
}

void UMultiplayGameServerSubsystem::OnPayloadToken(const Multiplay::PayloadTokenResponse& Response)
{
	FMultiplayPayloadTokenResponse MultiplayTokenResponseBodyStruct = {};

	if (Response.IsSuccessful())
	{
		UE_LOG(LogMultiplayGameServerSDK, Log, TEXT("PayloadToken() was successful"));
		const FHttpResponsePtr& HttpResponse = Response.GetHttpResponse();
		int32 ResponseCode = HttpResponse->GetResponseCode();
		FString ResponseBody = HttpResponse->GetContentAsString();
		TSharedPtr<FJsonValue> JsonParsed;
		TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(ResponseBody);
		
		if (FJsonSerializer::Deserialize(JsonReader, JsonParsed) && JsonParsed.IsValid())
		{
			TUniquePtr<Multiplay::OpenAPIPayloadTokenResponseBody> PayloadTokenResponseBody = MakeUnique<Multiplay::OpenAPIPayloadTokenResponseBody>();
			if (PayloadTokenResponseBody->FromJson(JsonParsed))
			{
				MultiplayTokenResponseBodyStruct.Error = PayloadTokenResponseBody->Error;
				MultiplayTokenResponseBodyStruct.Token = PayloadTokenResponseBody->Token;
				OnPayloadTokenSuccess.ExecuteIfBound(MultiplayTokenResponseBodyStruct);
			}
			else
			{
				MultiplayTokenResponseBodyStruct.Error = TEXT("Succeeded retrieving token but failed to parse the response");
				MultiplayTokenResponseBodyStruct.Token = TEXT("");
				OnPayloadTokenFailure.ExecuteIfBound(MultiplayTokenResponseBodyStruct);
			}
		}
		else 
		{
			MultiplayTokenResponseBodyStruct.Error = TEXT("Succeeded retrieving token but failed to deserialize the response");
			MultiplayTokenResponseBodyStruct.Token = TEXT("");
			OnPayloadTokenFailure.ExecuteIfBound(MultiplayTokenResponseBodyStruct);
		}
	}
	else
	{
		const FHttpResponsePtr& HttpResponse = Response.GetHttpResponse();
		int32 ResponseCode = HttpResponse->GetResponseCode();
		FString ResponseBody = HttpResponse->GetContentAsString();
		TSharedPtr<FJsonValue> JsonParsed;
		TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(ResponseBody);
		
		if (FJsonSerializer::Deserialize(JsonReader, JsonParsed) && JsonParsed.IsValid())
		{
			TUniquePtr<Multiplay::OpenAPIPayloadTokenResponseBody> ErrorResponseBody = MakeUnique<Multiplay::OpenAPIPayloadTokenResponseBody>();
			if (ErrorResponseBody->FromJson(JsonParsed))
			{
				MultiplayTokenResponseBodyStruct.Error = ErrorResponseBody->Error;
				MultiplayTokenResponseBodyStruct.Token = ErrorResponseBody->Token;
			}
			else
			{
				MultiplayTokenResponseBodyStruct.Error = TEXT("Failed to parse Json response body on failure");
				MultiplayTokenResponseBodyStruct.Token = TEXT("");
			}
		}
		else 
		{
			MultiplayTokenResponseBodyStruct.Error = TEXT("Failed to deserialize Json response body on failure");
			MultiplayTokenResponseBodyStruct.Token = TEXT("");
		}

		UE_LOG(LogMultiplayGameServerSDK, Error, TEXT("OnPayloadToken() was unsuccessful, response status code is '%d' and response body is '%s'"), ResponseCode, *ResponseBody);

		OnPayloadTokenFailure.ExecuteIfBound(MultiplayTokenResponseBodyStruct);
	}
}