#pragma once

#include "CoreMinimal.h"
#include "Utils/MultiplayJsonHelpers.h"

namespace Multiplay
{
	class FMultiplayServerJson : IJsonReadable
	{
	public:
		virtual ~FMultiplayServerJson();
		virtual bool FromJson(const TSharedPtr<FJsonValue>& JsonValue) override;

	public:
		int64 ServerId;
		FString AllocationId;
		uint16 QueryPort;
		uint16 Port;
		FString ServerLogDirectory;
	};
} // namespace Multiplay