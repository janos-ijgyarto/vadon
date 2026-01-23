using Sharpmake;

namespace Vadon.Utilities
{
    public static class ConfigurationParameters
    {
        public static string RepositoryPath { get; private set; }
        public static string GeneratorOutputPath { get; private set; }
        public static string BuildPath { get; private set; }
        public static string InstallPath { get; private set; }

        public static string FastBuildPath { get; private set; }

        public static string VcpkgPath { get; private set; }
        public static string ThirdPartyInstallPath { get; private set; }

        public static string QtPath { get; private set; }

        [CommandLine.Option("repositoryPath",
        @"Path to Vadon repository: ex: /repositoryPath(""path/to/vadon"")")]
        public static void CommandLineRepositoryPath(string path)
        {
            RepositoryPath = path;
        }

        [CommandLine.Option("generatorOutputPath",
        @"Path to Sharpmake output: ex: /generatorOutputPath(""path/to/generator/output"")")]
        public static void CommandLineGeneratorOutputPath(string path)
        {
            GeneratorOutputPath = path;
        }

        [CommandLine.Option("buildPath",
        @"Path to build output: ex: /buildPath(""path/to/build"")")]
        public static void CommandLineBuildOutputPath(string path)
        {
            BuildPath = path;
        }

        [CommandLine.Option("installPath",
        @"Path where engine modules can be installed: ex: /installPath(""path/to/install"")")]
        public static void CommandLineInstallPath(string path)
        {
            InstallPath = path;
        }

        [CommandLine.Option("fastBuildPath",
        @"Path to FastBuild executable: ex: /fastBuildPath(""path/to/fastbuild"")")]
        public static void CommandLineFastBuildPath(string path)
        {
            FastBuildPath = path;
        }

        [CommandLine.Option("vcpkgPath",
        @"Path to vcpkg root to manage dependencies: ex: /vcpkgPath(""path/to/vcpkg"")")]
        public static void CommandLineVcpkgPath(string path)
        {
            VcpkgPath = path;
        }

        [CommandLine.Option("thirdPartyInstallPath",
        @"Path where vcpkg can install dependencies: ex: /vcpkgPath(""path/to/third/party/install"")")]
        public static void CommandLineThirdPartyInstallPath(string path)
        {
            ThirdPartyInstallPath = path;
        }

        [CommandLine.Option("qtPath",
        @"Path to Qt: ex: /qtPath(""path/to/qt"")")]
        public static void CommandLineQtPath(string path)
        {
            QtPath = path;
        }

        public static void ValidateParameters()
        {
            if(string.IsNullOrEmpty(RepositoryPath))
            {
                throw new Sharpmake.Error("Must set valid repository path!");
            }

            if(string.IsNullOrEmpty(GeneratorOutputPath))
            {
                throw new Sharpmake.Error("Must set valid generator output path!");
            }

            if(string.IsNullOrEmpty(BuildPath))
            {
                throw new Sharpmake.Error("Must set valid build path!");
            }

            if(string.IsNullOrEmpty(InstallPath))
            {
                throw new Sharpmake.Error("Must set valid install path!");
            }

            if(string.IsNullOrEmpty(FastBuildPath))
            {
                throw new Sharpmake.Error("Must set valid FastBuild path!");
            }

            if(string.IsNullOrEmpty(VcpkgPath))
            {
                throw new Sharpmake.Error("Must set valid vcpkg path!");
            }

            if(string.IsNullOrEmpty(ThirdPartyInstallPath))
            {
                throw new Sharpmake.Error("Must set valid third party install path!");
            }

            if(string.IsNullOrEmpty(QtPath))
            {
                throw new Sharpmake.Error("Must set valid Qt path!");
            }
        }
    }
}