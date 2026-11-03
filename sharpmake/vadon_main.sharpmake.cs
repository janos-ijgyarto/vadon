using Sharpmake;
using System.IO;

[module: Sharpmake.Include("vadon.sharpmake.cs")]

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
			// When generating for engine development, use the C# file location
			// as the starting value for repository path
            Sharpmake.CommandLine.ExecuteOnType(typeof(Utilities.ConfigurationParameters), $"/vadonRepositoryPath(@'{GetRepositoryRootPath()}')");

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