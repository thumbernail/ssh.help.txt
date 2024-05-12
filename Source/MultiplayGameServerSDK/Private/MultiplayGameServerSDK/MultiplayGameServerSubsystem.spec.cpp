#include "Tests/AutomationCommon.h"
#include "Utils/AutomationTestUtils.h"
#include "Engine/World.h"
#include "MultiplayGameServerSubsystem.h"

#if WITH_AUTOMATION_TESTS

BEGIN_DEFINE_SPEC(FMultiplayGameServerSubsystemSpec, "MultiplayGameServerSDK.UMultiplayGameServerSubsystem", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ApplicationContextMask)
UMultiplayGameServerSubsystem* GameServerSubsystem;
END_DEFINE_SPEC(FMultiplayGameServerSubsystemSpec)

void FMultiplayGameServerSubsystemSpec::Define()
{
	BeforeEach([this]()
		{
			AutomationOpenMap(TEXT("/MultiplayGameServerSDK/Test"));

			UWorld* GameWorld = Multiplay::GetAnyGameWorld();
			UGameInstance* GameInstance = GameWorld->GetGameInstance();
			GameServerSubsystem = GameInstance->GetSubsystem<UMultiplayGameServerSubsystem>();
		});

	// TODO: Declare test cases for UMultiplayGameServerSubsystem.

	AfterEach([this]()
		{
			GameServerSubsystem = nullptr;
		});
}

#endif // #if WITH_AUTOMATION_TESTS
