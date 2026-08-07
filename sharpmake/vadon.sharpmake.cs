using Sharpmake;
using System.IO;

[module: Sharpmake.Include("application/application.sharpmake.cs")]
[module: Sharpmake.Include("demo/demo.sharpmake.cs")]
[module: Sharpmake.Include("editor/editor.sharpmake.cs")]
[module: Sharpmake.Include("engine/engine.sharpmake.cs")]
[module: Sharpmake.Include("foundation/foundation.sharpmake.cs")]
[module: Sharpmake.Include("tools/tools.sharpmake.cs")]
[module: Sharpmake.Include("utilities/configuration_parameters.sharpmake.cs")]
[module: Sharpmake.Include("utilities/qt.sharpmake.cs")]
[module: Sharpmake.Include("utilities/utils.sharpmake.cs")]

namespace Vadon
{
	public static class Main
	{
		private static void ConfigureAutoCleanup()
        {
            Util.FilesAutoCleanupActive = true;
            Util.FilesAutoCleanupDBPath = Path.Combine(Utilities.ConfigurationParameters.GeneratorOutputPath, "sharpmake");

            if (!Directory.Exists(Util.FilesAutoCleanupDBPath))
                Directory.CreateDirectory(Util.FilesAutoCleanupDBPath);
        }

		[Sharpmake.Main]
		public static void SharpmakeMain(Sharpmake.Arguments arguments)
		{
			Sharpmake.CommandLine.ExecuteOnType(typeof(Utilities.UserConfigurationFile));
            Sharpmake.CommandLine.ExecuteOnType(typeof(Utilities.ConfigurationParameters));
            Utilities.ConfigurationParameters.ValidateParameters();

			ConfigureAutoCleanup();

            FastBuildSettings.FastBuildMakeCommand = Utilities.ConfigurationParameters.FastBuildPath;
			// FIXME: this needs to be added because otherwise the generated BFFs
			// will not have the WinSDK path
			KitsRootPaths.SetUseKitsRootForDevEnv(DevEnv.vs2022, KitsRootEnum.KitsRoot10, Options.Vc.General.WindowsTargetPlatformVersion.v10_0_26100_0);

			// TODO: as a post-generation step, also generate scripts that will
			// install all of the third-party dependencies for all modules
			arguments.Generate<Engine.Solution>();
			arguments.Generate<Tools.Solution>();
		}

		public static string GetRepositoryRootPath()
		{
			string thisFilePath = Vadon.Utilities.UtilityFunctions.GetThisFilePath();
			return System.IO.Directory.GetParent(Path.GetDirectoryName(thisFilePath)).FullName;
		}
	}
}