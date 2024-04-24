// Some copyright should be here...

using System;
using System.IO;
using UnrealBuildTool;
public class FF_HTTP_Basic : ModuleRules
{
	public FF_HTTP_Basic(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        bEnableUndefinedIdentifierWarnings = false;
        bEnableExceptions = true;
        bUseRTTI = true;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "zlib",
                "OpenSSL",      // We don't need OpenSSL certificate functions
                "nghttp2",      // We need it for LibCurl
                "libcurl",
            }
            );
			
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
                "HTTP",
                "HTTPServer",
            }
			);
	}
}