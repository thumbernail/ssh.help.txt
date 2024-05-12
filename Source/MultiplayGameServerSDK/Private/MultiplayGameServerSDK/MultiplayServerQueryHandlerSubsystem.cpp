#include "MultiplayServerQueryHandlerSubsystem.h"
#include "Engine/GameInstance.h"
#include "Subsystems/SubsystemCollection.h"
#include "Serialization/ArrayWriter.h"
#include "MultiplayServerQueryProtocol.h"
#include "MultiplayServerConfigSubsystem.h"
#include "MultiplayGameServerSDKLog.h"

void UMultiplayServerQueryHandlerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// This subsystem is dependent on the server.json file having been parsed so that the queryPort value can be retrieved.
	Collection.InitializeDependency(UMultiplayServerConfigSubsystem::StaticClass());
}

void UMultiplayServerQueryHandlerSubsystem::Deinitialize()
{
	Disconnect();

	Super::Deinitialize();
}


bool UMultiplayServerQueryHandlerSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	// TODO: Provide a way to conditionally disable this subsystem outside of Multiplay.
	return true;
}

bool UMultiplayServerQueryHandlerSubsystem::Connect()
{
	if (IsConnected())
	{
		return true;
	}

    UMultiplayServerConfigSubsystem* Subsystem = GetGameInstance()->GetSubsystem<UMultiplayServerConfigSubsystem>();
    const FMultiplayServerConfig& ServerConfig = Subsystem->GetServerConfig();
	int32 QueryPort = ServerConfig.QueryPort;

	int32 BufferSize = 2 * 1024 * 1024;

	QuerySocket = FUdpSocketBuilder(TEXT("GameServerQueryReceiver"))
		.AsNonBlocking()
		.AsReusable()
		.BoundToAddress(FIPv4Address::Any)
		.BoundToPort(QueryPort)
		.WithSendBufferSize(BufferSize)
		.WithReceiveBufferSize(BufferSize);

	if (nullptr == QuerySocket)
	{
		UE_LOG(LogMultiplayGameServerSDK, Error, TEXT("Failed to bind socket to port '%u'"), QueryPort);
		return false;
	}

	FTimespan ThreadWaitTime = FTimespan::FromMilliseconds(100);
	UDPReceiver = MakeUnique<FUdpSocketReceiver>(QuerySocket, ThreadWaitTime, TEXT("QUERY_RECEIVER"));
	UDPReceiver->OnDataReceived().BindUObject(this, &UMultiplayServerQueryHandlerSubsystem::ReceiveSQPData);
	UDPReceiver->Start();

	UE_LOG(LogMultiplayGameServerSDK, Log, TEXT("Listening on port '%u'"), QueryPort);

	return true;
}

void UMultiplayServerQueryHandlerSubsystem::Disconnect()
{
	if (nullptr != UDPReceiver)
	{
		UDPReceiver = nullptr;
	}

	if (nullptr != QuerySocket)
	{
		QuerySocket->Close();

		ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
		if (nullptr != SocketSubsystem)
		{
			SocketSubsystem->DestroySocket(QuerySocket);
		}

		QuerySocket = nullptr;
	}
}

bool UMultiplayServerQueryHandlerSubsystem::IsConnected() const
{
	return (nullptr != UDPReceiver);
}

const int32& UMultiplayServerQueryHandlerSubsystem::GetCurrentPlayers() const
{ 
	return CurrentPlayers; 
}

void UMultiplayServerQueryHandlerSubsystem::DecrementCurrentPlayers() 
{
	if (CurrentPlayers > TNumericLimits<uint16>::Min())
	{ 
		CurrentPlayers -= 1;
	}
	else
	{
		UE_LOG(LogMultiplayGameServerSDK, Warning, TEXT("Cannot decrement CurrentPlayers below UINT16_MIN."));
	}
}

void UMultiplayServerQueryHandlerSubsystem::IncrementCurrentPlayers() 
{
	if (CurrentPlayers < TNumericLimits<uint16>::Max())
	{
		CurrentPlayers += 1;
	}
	else
	{
		UE_LOG(LogMultiplayGameServerSDK, Warning, TEXT("Cannot increment CurrentPlayers above UINT16_MAX."))
	}
}

void UMultiplayServerQueryHandlerSubsystem::SetCurrentPlayers(int32 Value)
{ 
	if ((TNumericLimits<uint16>::Max() < Value) || (TNumericLimits<uint16>::Min() > Value))
	{
		UE_LOG(LogMultiplayGameServerSDK, Warning, TEXT("Attempted to assign a value that cannot be represented by a uint16 to CurrentPlayers, it will be ignored"));
		return;
	}

	CurrentPlayers = Value;
}

const int32& UMultiplayServerQueryHandlerSubsystem::GetMaxPlayers() const
{ 
	return MaxPlayers; 
}

void UMultiplayServerQueryHandlerSubsystem::SetMaxPlayers(int32 Value) 
{ 
	if ((TNumericLimits<uint16>::Max() < Value) || (TNumericLimits<uint16>::Min() > Value))
	{
		UE_LOG(LogMultiplayGameServerSDK, Warning, TEXT("Attempted to assign a value that cannot be represented by a uint16 to MaxPlayers, it will be ignored"));
		return;
	}

	MaxPlayers = Value;
}

const FString& UMultiplayServerQueryHandlerSubsystem::GetServerName() const 
{ 
	return ServerName; 
}

void UMultiplayServerQueryHandlerSubsystem::SetServerName(FString Value) 
{ 
	FTCHARToUTF8 ConvertedString(*Value);

	if (ConvertedString.Length() > UMultiplayServerQueryHandlerSubsystem::kMaxStringLength)
	{
		UE_LOG(LogMultiplayGameServerSDK, Warning, TEXT("Attempted to assign a string longer than 255 characters to ServerName, which is unsupported by the server query protocol, this value will be ignored"));
		return;
	}

	ServerName = Value; 
}

const FString& UMultiplayServerQueryHandlerSubsystem::GetGameType() const 
{ 
	return GameType; 
}

void UMultiplayServerQueryHandlerSubsystem::SetGameType(FString Value) 
{ 
	FTCHARToUTF8 ConvertedString(*Value);

	if (ConvertedString.Length() > UMultiplayServerQueryHandlerSubsystem::kMaxStringLength)
	{
		UE_LOG(LogMultiplayGameServerSDK, Warning, TEXT("Attempted to assign a string longer than 255 characters to GameType, which is unsupported by the server query protocol, this value will be ignored"));
		return;
	}

	GameType = Value;
}

const FString& UMultiplayServerQueryHandlerSubsystem::GetBuildId() const 
{ 
	return BuildId; 
}

void UMultiplayServerQueryHandlerSubsystem::SetBuildId(FString Value) 
{ 
	FTCHARToUTF8 ConvertedString(*Value);

	if (ConvertedString.Length() > UMultiplayServerQueryHandlerSubsystem::kMaxStringLength)
	{
		UE_LOG(LogMultiplayGameServerSDK, Warning, TEXT("Attempted to assign a string longer than 255 characters to BuildId, which is unsupported by the server query protocol, this value will be ignored"));
		return;
	}

	BuildId = Value; 
}

const FString& UMultiplayServerQueryHandlerSubsystem::GetMap() const 
{ 
	return Map; 
}

void UMultiplayServerQueryHandlerSubsystem::SetMap(FString Value) 
{ 
	FTCHARToUTF8 ConvertedString(*Value);

	if (ConvertedString.Length() > UMultiplayServerQueryHandlerSubsystem::kMaxStringLength)
	{
		UE_LOG(LogMultiplayGameServerSDK, Warning, TEXT("Attempted to assign a string longer than 255 characters to Map, which is unsupported by the server query protocol, this value will be ignored"));
		return;
	}

	Map = Value;
}

const int32& UMultiplayServerQueryHandlerSubsystem::GetPort() const 
{ 
	return Port; 
}

void UMultiplayServerQueryHandlerSubsystem::SetPort(int32 Value) 
{ 
	if ((TNumericLimits<uint16>::Max() < Value) || (TNumericLimits<uint16>::Min() > Value))
	{
		UE_LOG(LogMultiplayGameServerSDK, Warning, TEXT("Attempted to assign a value that cannot be represented by a uint16 to Port, it will be ignored"));
		return;
	}

	Port = Value; 
}

void UMultiplayServerQueryHandlerSubsystem::ReceiveSQPData(const FArrayReaderPtr& ArrayReaderPtr, const FIPv4Endpoint& EndPt)
{
	if (!QuerySocket)
	{
		UE_LOG(LogMultiplayGameServerSDK, Warning, TEXT("No query socket"));
		return;
	}

	// We only need the header at this stage
	Multiplay::FSQPHeader Header = {};
	*ArrayReaderPtr << Header;

	TSharedRef<FInternetAddr> FromAddress = EndPt.ToInternetAddr();

	// Don't proceed if we've received a packet that's too small
	if (ArrayReaderPtr->TotalSize() < 5)
	{
		UE_LOG(LogMultiplayGameServerSDK, Warning, TEXT("Received a packet that was too small"));
		return;
	}

	switch (Header.Type)
	{
	case static_cast<uint8>(Multiplay::ESQPMessageType::ChallengeRequest):
	{
		UE_LOG(LogMultiplayGameServerSDK, Verbose, TEXT("Received ChallengeRequest packet."))

		SendSQPChallengePacket(FromAddress);

		break;
	}
	case static_cast<uint8>(Multiplay::ESQPMessageType::QueryRequest):
	{
		UE_LOG(LogMultiplayGameServerSDK, Verbose, TEXT("Received QueryRequest packet."))

		SendSQPQueryPacket(ArrayReaderPtr, FromAddress);
		break;
	}
	default:
	{
		UE_LOG(LogMultiplayGameServerSDK, Warning, TEXT("Received unrecognized packet type: %u"), Header.Type);
		break;
	}
	}
}

void UMultiplayServerQueryHandlerSubsystem::SendSQPChallengePacket(TSharedRef<FInternetAddr> FromAddress)
{
	// Only issue a challenge token if we have no pending requests from this address
	FString FromAddressString = FromAddress->ToString(true);
	if (!FSQPChallengeTokens.Contains(FromAddressString))
	{
		// We will cast this to a uint32 later
		int32 NextToken = FMath::RandHelper(INT32_MAX);

		// Construct the challenge response packet
		Multiplay::FSQPChallengePacket ChallengePacket = {};
		ChallengePacket.Header.Type = static_cast<uint8>(Multiplay::ESQPMessageType::ChallengeResponse);
		ChallengePacket.Header.ChallengeToken = NextToken;

		// Serialize the challenge response packet
		FArrayWriter Writer;
		Writer << ChallengePacket;

		// Send the packet to the address that requested it
		int32 BytesSent = 0;
		QuerySocket->SendTo(Writer.GetData(), Writer.Num(), BytesSent, *FromAddress);

		if (BytesSent <= 0)
		{
			UE_LOG(LogMultiplayGameServerSDK, Warning, TEXT("Socket is valid but the receiver received 0 bytes, make sure it is listening properly!"));
		}

		// Store the challenge request token, we will use this to validate subsequent requests
		FSQPChallengeTokens.Add(FromAddressString, NextToken);
	}
	else
	{
		UE_LOG(LogMultiplayGameServerSDK, Warning, TEXT("Address already found"));
	}
}

void UMultiplayServerQueryHandlerSubsystem::SendSQPQueryPacket(const FArrayReaderPtr& ArrayReaderPtr, TSharedRef<FInternetAddr> FromAddress)
{
	FString FromAddressString = FromAddress->ToString(true);

	// If this address has not first requested a challenge packet, do not proceed
	if (!FSQPChallengeTokens.Contains(FromAddressString))
	{
		UE_LOG(LogMultiplayGameServerSDK, Warning, TEXT("Attempting to query from an address that did not receive a challenge token"));
		return;
	}

	// We have already read the header, reset our read position to the beginning of the packet
	ArrayReaderPtr->Seek(0);
	Multiplay::FSQPQueryRequestPacket QueryRequestPacket = {};
	*ArrayReaderPtr << QueryRequestPacket;

	// Ensure this request has a matching challenge token to what we're expecting
	uint32 StoredChallengeToken = 0;
	FSQPChallengeTokens.RemoveAndCopyValue(FromAddressString, StoredChallengeToken);
	if (QueryRequestPacket.Header.ChallengeToken != StoredChallengeToken)
	{
		UE_LOG(LogMultiplayGameServerSDK, Warning, TEXT("Received challenge token (%d) does not match stored challenge token (%d)"), QueryRequestPacket.Header.ChallengeToken, StoredChallengeToken);
		return;
	}

	// Start to construct the response packet
	Multiplay::FSQPQueryResponsePacket ResponsePacket = {};
	ResponsePacket.RequestedChunks = QueryRequestPacket.RequestedChunks;

	// Start with the generic header information
	ResponsePacket.QueryHeader.Header.Type = static_cast<uint8>(Multiplay::ESQPMessageType::QueryResponse);
	ResponsePacket.QueryHeader.Header.ChallengeToken = QueryRequestPacket.Header.ChallengeToken;
	ResponsePacket.QueryHeader.CurrentPacket = 0;
	ResponsePacket.QueryHeader.LastPacket = 0;

	// Now add the query header information
	ResponsePacket.QueryHeader.Version = QueryRequestPacket.Version;

	// We are only required to respond to requests for ServerInfo
	if ((QueryRequestPacket.RequestedChunks & static_cast<uint8>(Multiplay::ESQPChunkType::ServerInfo)) > 0)
	{
		ResponsePacket.ServerInfoData.CurrentPlayers = CurrentPlayers;
		ResponsePacket.ServerInfoData.MaxPlayers = MaxPlayers;
		ResponsePacket.ServerInfoData.ServerName = ServerName;
		ResponsePacket.ServerInfoData.GameType = GameType;
		ResponsePacket.ServerInfoData.BuildId = BuildId;
		ResponsePacket.ServerInfoData.Map = Map;
		ResponsePacket.ServerInfoData.Port = Port;
	}

	// Serialize the query response packet
	FArrayWriter Writer;
	Writer << ResponsePacket;

	// Send the packet to the address that requested it
	int32 BytesSent = 0;
	QuerySocket->SendTo(Writer.GetData(), Writer.Num(), BytesSent, *FromAddress);

	if (BytesSent <= 0)
	{
		UE_LOG(LogMultiplayGameServerSDK, Warning, TEXT("Socket is valid but the receiver received 0 bytes, make sure it is listening properly!"));
	}
}
