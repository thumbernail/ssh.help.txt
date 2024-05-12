#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MultiplayServerConfig.h"
#include "MultiplayServerConfigSubsystem.generated.h"

/**
 * Subsystem responsible for retrieving the Multiplay server configuration.
 */
UCLASS()
class MULTIPLAYGAMESERVERSDK_API UMultiplayServerConfigSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    /**
     * @brief Accessor for the server configuration.
     * @return The server configuration for the current session.
     */
    const FMultiplayServerConfig& GetServerConfig() const;

private:
    /**
     * The server configuration for the current session.
     */
    UPROPERTY(BlueprintReadOnly, Category="Multiplay | ServerConfig", meta = (AllowPrivateAccess = "true"))
    FMultiplayServerConfig ServerConfig;
};
