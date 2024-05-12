
using UnrealBuildTool;

public class MultiplayGameServerSDK : ModuleRules
{
	public MultiplayGameServerSDK(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"Http",
				"Json",
				"JsonUtilities",
				"Networking",
				"Sockets",
				"WebSockets",
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
			}
			);
	}
}
