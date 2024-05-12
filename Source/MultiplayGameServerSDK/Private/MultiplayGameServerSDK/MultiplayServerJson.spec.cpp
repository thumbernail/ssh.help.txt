#include "Tests/AutomationCommon.h"
#include "MultiplayGameServerSDK/MultiplayServerJson.h"

#if WITH_AUTOMATION_TESTS

BEGIN_DEFINE_SPEC(FMultiplayServerJsonSpec, "MultiplayGameServerSDK.FMultiplayServerJson", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ApplicationContextMask)
END_DEFINE_SPEC(FMultiplayServerJsonSpec)

void FMultiplayServerJsonSpec::Define()
{
	Describe("FromJson", [this]()
		{
			It("should return true when serverID, queryPort, and port are integers.", [this]()
				{
					int64 ServerId = 12345;
					FString AllocationId = TEXT("AAAAAAAA-BBBB-CCCC-DDDDD-EEEEEEEEEEEE");
					uint16 QueryPort = 7778;
					uint16 Port = 7777;
					FString ServerLogDirectory = TEXT("/home");

					FString JsonString = FString::Printf(TEXT("{\"serverID\": %d, \"allocatedUUID\": \"%s\", \"queryPort\": %d, \"port\": %d, \"serverLogDir\": \"%s\"}"), ServerId, *AllocationId, QueryPort, Port, *ServerLogDirectory);

					TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<>::Create(JsonString);

					TSharedPtr<FJsonValue> JsonValue;
					FJsonSerializer::Deserialize(JsonReader, JsonValue);

					Multiplay::FMultiplayServerJson ServerJson;
					bool bDidParseJson = ServerJson.FromJson(JsonValue);

					TestTrue(TEXT("bDidParseJson is true"), bDidParseJson);
					TestEqual(TEXT("FMultiplayServerJson::ServerId has the correct value"), ServerJson.ServerId, ServerId);
					TestEqual(TEXT("FMultiplayServerJson::AllocationId has the correct value"), ServerJson.AllocationId, AllocationId);
					TestEqual(TEXT("FMultiplayServerJson::QueryPort has the correct value"), ServerJson.QueryPort, QueryPort);
					TestEqual(TEXT("FMultiplayServerJson::Port has the correct value"), ServerJson.Port, Port);
					TestEqual(TEXT("FMultiplayServerJson::ServerLogDirectory has the correct value"), ServerJson.ServerLogDirectory, ServerLogDirectory);
				});

            It("should return true when serverID, queryPort, and port are strings.", [this]()
                {
                    int64 ServerId = 12345;
                    FString AllocationId = TEXT("AAAAAAAA-BBBB-CCCC-DDDDD-EEEEEEEEEEEE");
                    uint16 QueryPort = 7778;
                    uint16 Port = 7777;
                    FString ServerLogDirectory = TEXT("/home");

                    FString JsonString = FString::Printf(TEXT("{\"serverID\": \"%d\", \"allocatedUUID\": \"%s\", \"queryPort\": \"%d\", \"port\": \"%d\", \"serverLogDir\": \"%s\"}"), ServerId, *AllocationId, QueryPort, Port, *ServerLogDirectory);

                    TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<>::Create(JsonString);

                    TSharedPtr<FJsonValue> JsonValue;
                    FJsonSerializer::Deserialize(JsonReader, JsonValue);

                    Multiplay::FMultiplayServerJson ServerJson;
                    bool bDidParseJson = ServerJson.FromJson(JsonValue);

                    TestTrue(TEXT("bDidParseJson is true"), bDidParseJson);
                    TestEqual(TEXT("FMultiplayServerJson::ServerId has the correct value"), ServerJson.ServerId, ServerId);
                    TestEqual(TEXT("FMultiplayServerJson::AllocationId has the correct value"), ServerJson.AllocationId, AllocationId);
                    TestEqual(TEXT("FMultiplayServerJson::QueryPort has the correct value"), ServerJson.QueryPort, QueryPort);
                    TestEqual(TEXT("FMultiplayServerJson::Port has the correct value"), ServerJson.Port, Port);
                    TestEqual(TEXT("FMultiplayServerJson::ServerLogDirectory has the correct value"), ServerJson.ServerLogDirectory, ServerLogDirectory);
                });
		});
}

#endif // #if WITH_AUTOMATION_TESTS
