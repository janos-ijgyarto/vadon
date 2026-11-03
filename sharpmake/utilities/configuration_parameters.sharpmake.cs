using Sharpmake;
using System;
using System.Collections.Generic;

namespace Vadon.Utilities
{
    public static class UserConfigurationFile
    {
        public static string ConfigFilePath { get; private set; }

        [CommandLine.Option("vadonUserConfigPath",
        @"Path to config file that contains parameters: ex: /userConfigPath(""path/to/user_config.json"")")]
        public static void CommandLineUserConfigPath(string path)
        {
            ConfigFilePath = path;
        }

        // FIXME: this is a bit hacky
        // Find a more robust way to allow params with multiple args
        // and then feed into the same interface as the command line args
        public static Dictionary<string, string> GetConfigParameters()
        { 
            if(string.IsNullOrEmpty(ConfigFilePath))
            {
                return null;
            }

            if(System.IO.File.Exists(ConfigFilePath) == false)
            {
                return null;
            }

            Dictionary<string,string> parameters = new Dictionary<string,string>();

            ReadOnlySpan<byte> jsonReadOnlySpan = System.IO.File.ReadAllBytes(ConfigFilePath);
            if(jsonReadOnlySpan.Length == 0)
            {
                return null;
            }

            var reader = new System.Text.Json.Utf8JsonReader(jsonReadOnlySpan);
            while (reader.Read())
            {
                switch (reader.TokenType)
                {
                    case System.Text.Json.JsonTokenType.PropertyName:
                        string propertyName = reader.GetString();
                        reader.Read();
                        switch (reader.TokenType)
                        {
                            case System.Text.Json.JsonTokenType.String:
                                {
                                    parameters.Add(propertyName, $"@'{reader.GetString()}'");
                                }
                                break;
                            case System.Text.Json.JsonTokenType.StartArray:
                                {         
                                    reader.Read();
                                    List<string> stringList = new List<string>();
                                    while(reader.TokenType != System.Text.Json.JsonTokenType.EndArray)
                                    {                            
                                        if(reader.TokenType == System.Text.Json.JsonTokenType.String)
                                        {
                                            stringList.Add($"@'{reader.GetString()}");
                                            reader.Read();
                                        }
                                        else
                                        {
                                            reader.Skip();
                                        }
                                    }
                                    parameters.Add(propertyName, string.Join(",", stringList));
                                }
                                break;
                        }
                        break;
                }
            }

            return parameters;
        }
    }

    public static class ConfigurationParameters
    {
        public static string RepositoryPath { get; private set; }

        public static string OutputRootPath { get; private set; }
        public static string GeneratorOutputPath { get; private set; }
        public static string BuildPath { get; private set; }

        public static string FastBuildPath { get; private set; }

        public static string VcpkgPath { get; private set; }
        public static string ThirdPartyInstallPath { get; private set; }

        public static string QtPath { get; private set; }

        [CommandLine.Option("vadonRepositoryPath",
        @"Path to Vadon repository: ex: /vadonRepositoryPath(""path/to/vadon"")")]
        public static void CommandLineRepositoryPath(string path)
        {
            RepositoryPath = path;
        }

        [CommandLine.Option("vadonOutputRootPath",
        @"Root path to build system output: ex: /vadonOutputRootPath(""path/to/output"")")]
        public static void CommandOutputRootPath(string path)
        {
            OutputRootPath = path;
        }

        [CommandLine.Option("vadonGeneratorOutputPath",
        @"Path to Sharpmake generator output: ex: /vadonGeneratorOutputPath(""path/to/generator/output"")")]
        public static void CommandLineGeneratorOutputPath(string path)
        {
            GeneratorOutputPath = path;
        }

        [CommandLine.Option("vadonBuildPath",
        @"Path to build output: ex: /vadonBuildPath(""path/to/build"")")]
        public static void CommandLineBuildOutputPath(string path)
        {
            BuildPath = path;
        }

        [CommandLine.Option("vadonFastBuildPath",
        @"Path to FastBuild executable: ex: /vadonFastBuildPath(""path/to/fastbuild"")")]
        public static void CommandLineFastBuildPath(string path)
        {
            FastBuildPath = path;
        }

        [CommandLine.Option("vadonVcpkgPath",
        @"Path to vcpkg root to manage dependencies: ex: /vadonVcpkgPath(""path/to/vcpkg"")")]
        public static void CommandLineVcpkgPath(string path)
        {
            VcpkgPath = path;
        }

        [CommandLine.Option("vadonThirdPartyInstallPath",
        @"Path where vcpkg can install dependencies: ex: /vadonThirdPartyInstallPath(""path/to/third/party/install"")")]
        public static void CommandLineThirdPartyInstallPath(string path)
        {
            ThirdPartyInstallPath = path;
        }

        [CommandLine.Option("vadonQtPath",
        @"Path to Qt: ex: /vadonQtPath(""path/to/qt"")")]
        public static void CommandLineQtPath(string path)
        {
            QtPath = path;
        }

        private static void ProcessConfigFile()
        {
            // FIXME: this is a bit hacky, only works for methods with 1 string argument
            Dictionary<string, string> parameters = UserConfigurationFile.GetConfigParameters();
            if(parameters == null)
            {
                return;
            }
            if(parameters.Count == 0)
            {
                return;
            }

            foreach(var (key, value) in parameters)
            {
                string commandString = $"/{key}";
                if(string.IsNullOrEmpty(value) == false)
                {
                    commandString += $"({value})";
                }
                Sharpmake.CommandLine.ExecuteOnType(typeof(ConfigurationParameters), commandString);
            }
        }

        public static void ValidateParameters()
        {
            // First process the config file, then override using command line args
            ProcessConfigFile();

            if(string.IsNullOrEmpty(RepositoryPath))
            {
                throw new Sharpmake.Error("Must set valid repository path!");
            }

            if (string.IsNullOrEmpty(FastBuildPath))
            {
                throw new Sharpmake.Error("Must set valid FastBuild path!");
            }
            // TODO: confirm that FASTBuild is at the path

            if(string.IsNullOrEmpty(VcpkgPath))
            {
                throw new Sharpmake.Error("Must set valid vcpkg path!");
            }
            // TODO: confirm that vcpkg is at the path

            if (string.IsNullOrEmpty(QtPath))
            {
                throw new Sharpmake.Error("Must set valid Qt path!");
            }
            // TODO: confirm that Qt is installed at the path

            if(string.IsNullOrEmpty(OutputRootPath))
            {
                OutputRootPath = System.IO.Path.Combine(RepositoryPath, "output");
            }

            if(string.IsNullOrEmpty(GeneratorOutputPath))
            {
                GeneratorOutputPath = System.IO.Path.Combine(OutputRootPath, "generator");
            }

            if (string.IsNullOrEmpty(BuildPath))
            {
                BuildPath = System.IO.Path.Combine(OutputRootPath, "build");
            }

            if (string.IsNullOrEmpty(ThirdPartyInstallPath))
            {
                ThirdPartyInstallPath = System.IO.Path.Combine(OutputRootPath, "third_party");
            }

            try
            {
                System.IO.Directory.CreateDirectory(GeneratorOutputPath);
                System.IO.Directory.CreateDirectory(BuildPath);
                System.IO.Directory.CreateDirectory(ThirdPartyInstallPath);
            }
            catch(System.Exception e)
            {
                throw new Sharpmake.Error($"Failed to create output paths! Error: {e.Message}");
            }
        }
    }
}