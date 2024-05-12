#include "AutomationTestUtils.h"

#ifdef WITH_AUTOMATION_TESTS

#include "Engine/Engine.h"
#include "Engine/World.h"

namespace Multiplay
{
	// Copied from Engine/Source/Runtime/Engine/Private/Tests/AutomationCommon.cpp.
	// This was the only example I could find demonstrating how to obtain a UWorld reference inside of an automation test.
	UWorld* GetAnyGameWorld()
	{
		UWorld* TestWorld = nullptr;
		const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
		for (const FWorldContext& Context : WorldContexts)
		{
			if (((Context.WorldType == EWorldType::PIE) || (Context.WorldType == EWorldType::Game)) && (Context.World() != NULL))
			{
				TestWorld = Context.World();
				break;
			}
		}

		return TestWorld;
	}
} // namespace Multiplay

#endif // #ifdef WITH_AUTOMATION_TESTS
