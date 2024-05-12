#include "MultiplayCentrifugeMessages.h"
#include "Tests/AutomationCommon.h"
#include "Utils/AutomationTestUtils.h"

#if WITH_AUTOMATION_TESTS

BEGIN_DEFINE_SPEC(FMultiplayCentrifugeMessagesSpec, "MultiplayGameServerSDK.UMultiplayCentrifugeMessages", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ApplicationContextMask)
END_DEFINE_SPEC(FMultiplayCentrifugeMessagesSpec)

void FMultiplayCentrifugeMessagesSpec::Define()
{
	Describe("FError", [this]()
		{
			Describe("FromJson", [this]()
				{
					It("returns true if JSON data is parsed correctly.", [this]()
						{
							uint32 ErrorCode = 3003;
							FString ErrorMessage = TEXT("Hello, World!");
							FString JsonString = FString::Printf(TEXT(R"({"code": %d, "message": "%s"})"), ErrorCode, *ErrorMessage);

							TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<>::Create(JsonString);

							bool bDidParseJson;
							Multiplay::FError Error;
							TSharedPtr<FJsonValue> JsonValue;
							if (FJsonSerializer::Deserialize(JsonReader, JsonValue) && JsonValue.IsValid())
							{
								bDidParseJson = Error.FromJson(JsonValue);
							}
							else
							{
								bDidParseJson = false;
							}

							TestTrueExpr(bDidParseJson);
							TestEqual("FError::Code", Error.Code, ErrorCode);
							TestEqual("FError::Message", Error.Message, ErrorMessage);
						});
				});
		});

	Describe("FConnectResult", [this]()
		{
			Describe("FromJson", [this]()
				{
					It("returns true if JSON data is parsed correctly.", [this]()
						{
							FString Client = TEXT("foo");
							FString Version = TEXT("bar");
							bool bExpires = true;
							uint32 TTL = 1000;
							FString JsonString = FString::Printf(TEXT(R"({"client": "%s", "version": "%s", "expires": %s, "ttl": %u})"), *Client, *Version, bExpires ? TEXT("true") : TEXT("false"), TTL);

							TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<>::Create(JsonString);

							bool bDidParseJson;
							Multiplay::FConnectResult Result;
							TSharedPtr<FJsonValue> JsonValue;
							if (FJsonSerializer::Deserialize(JsonReader, JsonValue) && JsonValue.IsValid())
							{
								bDidParseJson = Result.FromJson(JsonValue);
							}
							else
							{
								bDidParseJson = false;
							}

							TestTrueExpr(bDidParseJson);
							TestEqual("FConnectResult::Client", Result.Client.Get(TEXT("")), Client);
							TestEqual("FConnectResult::Version", Result.Version.Get(TEXT("")), Version);
							TestEqual("FConnectResult::bExpires", Result.bExpires.Get(false), bExpires);
							TestEqual("FConnectResult::TTL", Result.TTL.Get(0), TTL);
						});
				});
		});

	Describe("FMessage", [this]()
		{
			Describe("FromJson", [this]()
				{
					It("returns true if JSON data is parsed correctly.", [this]()
						{
							FString JsonString = TEXT(R"({"data": {} })");

							TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<>::Create(JsonString);

							bool bDidParseJson;
							Multiplay::FMessage Message;
							TSharedPtr<FJsonValue> JsonValue;
							if (FJsonSerializer::Deserialize(JsonReader, JsonValue) && JsonValue.IsValid())
							{
								bDidParseJson = Message.FromJson(JsonValue);
							}
							else
							{
								bDidParseJson = false;
							}

							TestTrueExpr(bDidParseJson);

							if (MP_TEST_TRUE_EXPR(Message.Data.IsSet()))
							{
								TSharedPtr<FJsonValue> Data = Message.Data.GetValue();

								TestEqual("FJsonValue::Type", Data->Type, EJson::Object);
							}
						});
				});
		});

	Describe("FConnect", [this]()
		{
			Describe("FromJson", [this]()
				{
					It("returns true if JSON data is parsed correctly.", [this]()
						{
							FString Client = TEXT("foo");
							FString Version = TEXT("bar");
							bool bExpires = true;
							uint32 TTL = 1000;
							FString JsonString = FString::Printf(TEXT(R"({"client": "%s", "version": "%s", "expires": %s, "ttl": %u})"), *Client, *Version, bExpires ? TEXT("true") : TEXT("false"), TTL);

							TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<>::Create(JsonString);

							bool bDidParseJson;
							Multiplay::FConnect Result;
							TSharedPtr<FJsonValue> JsonValue;
							if (FJsonSerializer::Deserialize(JsonReader, JsonValue) && JsonValue.IsValid())
							{
								bDidParseJson = Result.FromJson(JsonValue);
							}
							else
							{
								bDidParseJson = false;
							}

							TestTrueExpr(bDidParseJson);
							TestEqual("FConnect::Client", Result.Client, Client);
							TestEqual("FConnect::Version", Result.Version, Version);
							TestEqual("FConnect::bExpires", Result.bExpires, bExpires);
							TestEqual("FConnect::TTL", Result.TTL, TTL);
						});
				});
		});

	Describe("FDisconnect", [this]()
		{
			Describe("FromJson", [this]()
				{
					It("returns true if JSON data is parsed correctly.", [this]()
						{
							uint32 Code = static_cast<uint32>(Multiplay::EDisconnectCode::DisconnectForceReconnect);
							FString Reason = TEXT("Lorem ipsum dolor sit amet, consectetur adipiscing elit.");
							bool bReconnect = true;
							FString JsonString = FString::Printf(TEXT(R"({"code": %d, "reason": "%s", "reconnect": %s})"), Code, *Reason, bReconnect ? TEXT("true") : TEXT("false"));

							TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<>::Create(JsonString);

							bool bDidParseJson;
							Multiplay::FDisconnect Disconnect;
							TSharedPtr<FJsonValue> JsonValue;
							if (FJsonSerializer::Deserialize(JsonReader, JsonValue) && JsonValue.IsValid())
							{
								bDidParseJson = Disconnect.FromJson(JsonValue);
							}
							else
							{
								bDidParseJson = false;
							}

							TestTrueExpr(bDidParseJson);
							TestEqual("FDisconnect::Code", Disconnect.Code, Code);
							TestEqual("FDisconnect::Reason", Disconnect.Reason, Reason);
							TestEqual("FDisconnect::bReconnect", Disconnect.bReconnect, bReconnect);
						});
				});
		});

	Describe("FPingResult", [this]()
		{
			Describe("FromJson", [this]()
				{
					It("should return true.", [this]()
						{
							FString JsonString = TEXT("{}");

							TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<>::Create(JsonString);

							bool bDidParseJson;
							Multiplay::FPingResult PingResult;
							TSharedPtr<FJsonValue> JsonValue;
							if (FJsonSerializer::Deserialize(JsonReader, JsonValue) && JsonValue.IsValid())
							{
								bDidParseJson = PingResult.FromJson(JsonValue);
							}
							else
							{
								bDidParseJson = false;
							}

							TestTrueExpr(bDidParseJson);
						});
				});
		});

	Describe("FCommand", [this]()
		{
			Describe("WriteJson", [this]()
				{
					It("returns true if JSON writer populates data.", [this]()
						{
							Multiplay::FConnectRequest ConnectRequest = {};
							ConnectRequest.Token = FString(TEXT("foo"));
							ConnectRequest.Name = FString(TEXT("bar"));
							ConnectRequest.Version = FString(TEXT("baz"));

							auto Command = MakeUnique<Multiplay::FCommand>(12345, MakeUnique<Multiplay::FConnectRequest>(ConnectRequest));

							FString JsonBody;

							Multiplay::JsonWriter Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&JsonBody);

							Command->WriteJson(Writer);

							Writer->Close();

							TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<>::Create(JsonBody);
							TSharedPtr<FJsonObject> Object;
							if (MP_TEST_TRUE_EXPR(FJsonSerializer::Deserialize(JsonReader, Object)))
							{
								uint32 Id;
								if (MP_TEST_TRUE_EXPR(Object->TryGetNumberField(TEXT("id"), Id)))
								{
									TestEqual("FCommand::Id", Id, Command->GetId());
								}

								Multiplay::EMethodType Method;
								if (MP_TEST_TRUE_EXPR(Object->TryGetNumberField(TEXT("method"), (uint32&)Method)))
								{
									TestEqual("FCommand::Method", Method, Command->GetMethod());
								}

								const TSharedPtr<FJsonObject>& Params = Object->GetObjectField(TEXT("params"));
								if (MP_TEST_VALID("Params", Params))
								{
									FString Token;
									if (MP_TEST_TRUE_EXPR(Params->TryGetStringField(TEXT("token"), Token)))
									{
										TestEqual("FConnectRequest::Token", Token, ConnectRequest.Token.GetValue());
									}

									FString Name;
									if (MP_TEST_TRUE_EXPR(Params->TryGetStringField(TEXT("name"), Name)))
									{
										TestEqual("FConnectRequest::Name", Name, ConnectRequest.Name.GetValue());
									}

									FString Version;
									if (MP_TEST_TRUE_EXPR(Params->TryGetStringField(TEXT("version"), Version)))
									{
										TestEqual("FConnectRequest::Version", Version, ConnectRequest.Version.GetValue());
									}
								}
							}
						});
				});
		});

	Describe("FConnectRequest", [this]()
		{
			Describe("WriteJson", [this]()
				{
					It("returns true if JSON writer populates data.", [this]()
						{
							Multiplay::FConnectRequest ConnectRequest;
							ConnectRequest.Token = FString(TEXT("foo"));
							ConnectRequest.Name = FString(TEXT("bar"));
							ConnectRequest.Version = FString(TEXT("baz"));

							FString JsonBody;

							Multiplay::JsonWriter Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&JsonBody);

							ConnectRequest.WriteJson(Writer);

							Writer->Close();

							TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<>::Create(JsonBody);
							TSharedPtr<FJsonObject> Object;
							if (MP_TEST_TRUE_EXPR(FJsonSerializer::Deserialize(JsonReader, Object)))
							{
								FString Token;
								if (MP_TEST_TRUE_EXPR(Object->TryGetStringField(TEXT("token"), Token)))
								{
									TestEqual("FConnectRequest::Token", Token, ConnectRequest.Token.GetValue());
								}

								FString Name;
								if (MP_TEST_TRUE_EXPR(Object->TryGetStringField(TEXT("name"), Name)))
								{
									TestEqual("FConnectRequest::Name", Name, ConnectRequest.Name.GetValue());
								}

								FString Version;
								if (MP_TEST_TRUE_EXPR(Object->TryGetStringField(TEXT("version"), Version)))
								{
									TestEqual("FConnectRequest::Version", Version, ConnectRequest.Version.GetValue());
								}
							}
						});
				});
		});

	Describe("FSendRequest", [this]()
		{
			Describe("WriteJson", [this]()
				{
					It("returns true if JSON writer populates data.", [this]()
						{
							FString JsonBody;

							Multiplay::JsonWriter Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&JsonBody);

							Multiplay::FSendRequest SendRequest;
							SendRequest.WriteJson(Writer);

							Writer->Close();

							TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<>::Create(JsonBody);
							TSharedPtr<FJsonObject> Object;
							TestTrueExpr(FJsonSerializer::Deserialize(JsonReader, Object));
						});
				});
		});

	Describe("FPingRequest", [this]()
		{
			Describe("WriteJson", [this]()
				{
					It("returns true if JSON writer populates data.", [this]()
						{
							FString JsonBody;

							Multiplay::JsonWriter Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&JsonBody);

							Multiplay::FPingRequest PingRequest;
							PingRequest.WriteJson(Writer);

							Writer->Close();

							TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<>::Create(JsonBody);
							TSharedPtr<FJsonObject> Object;
							if (MP_TEST_TRUE_EXPR(FJsonSerializer::Deserialize(JsonReader, Object)))
							{
								TestTrueExpr(Object->Values.Num() == 0);
							}
						});
				});
		});
}

#endif // #if WITH_AUTOMATION_TESTS
