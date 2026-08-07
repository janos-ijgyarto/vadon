using Sharpmake;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;

namespace Vadon.Utilities
{
    public static class UserConfigurationFile
    {
        public static string ConfigFilePath { get; private set; }

        [CommandLine.Option("userConfigPath",
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
                                    parameters.Add(propertyName, $"\"{reader.GetString()}\"");
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
                                            stringList.Add($"\"{reader.GetString()}\"");
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

        [CommandLine.Option("repositoryPath",
        @"Path to Vadon repository: ex: /repositoryPath(""path/to/vadon"")")]
        public static void CommandLineRepositoryPath(string path)
        {
            RepositoryPath = path;
        }

        [CommandLine.Option("outputRootPath",
        @"Root path to build system output: ex: /outputRootPath(""path/to/output"")")]
        public static void CommandOutputRootPath(string path)
        {
            OutputRootPath = path;
        }

        [CommandLine.Option("generatorOutputPath",
        @"Path to Sharpmake generator output: ex: /generatorOutputPath(""path/to/generator/output"")")]
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
        @"Path where vcpkg can install dependencies: ex: /thirdPartyInstallPath(""path/to/third/party/install"")")]
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

        // NOTE: copied from Sharpmake.CommandLine.GetMethodsMapping
        private static Dictionary<string, List<MethodInfo>> GetMethodsMapping(Type type, bool isStatic)
        {
            Dictionary<string, List<MethodInfo>> results = new Dictionary<string, List<MethodInfo>>(StringComparer.OrdinalIgnoreCase);

            MethodInfo[] methodInfos = type.GetMethods();

            foreach (MethodInfo methodInfo in methodInfos)
            {
                if (methodInfo.IsStatic != isStatic)
                    continue;

                CommandLine.Option[] options = methodInfo.GetCustomAttributes(typeof(CommandLine.Option), false) as CommandLine.Option[];
                foreach (CommandLine.Option option in options)
                {
                    List<MethodInfo> optionsMethodInfo;
                    if (!results.TryGetValue(option.Name, out optionsMethodInfo))
                    {
                        optionsMethodInfo = new List<MethodInfo>();
                        results.Add(option.Name, optionsMethodInfo);
                    }
                    optionsMethodInfo.Add(methodInfo);
                }
            }
            return results;
        }

        // NOTE: implementation partly copied from Sharpmake.CommandLine.Execute
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

            Type type = typeof(ConfigurationParameters);
            var paramMethods = GetMethodsMapping(typeof(ConfigurationParameters), true);

            StringBuilder errors = new StringBuilder();

            // use associated assembly
            var assemblies = new List<Assembly> { type.Assembly };

            foreach(var (key, value) in parameters)
            {
                List<MethodInfo> methodInfos;
                if (paramMethods.TryGetValue(key, out methodInfos))
                {
                    foreach (MethodInfo methodInfo in methodInfos)
                    {
                        try
                        {
                            string executeCode = string.Format("{0}.{1}({2});", type.FullName.Replace("+", "."), methodInfo.Name, value);
                            Action execute = Assembler.BuildDelegate<Action>(executeCode, type.Namespace, CommandLine.DefaultNamespaces.ToArray(), assemblies.ToArray());

                            execute();
                        }
                        catch (Error e)
                        {
                            string[] parametersName = methodInfo.GetParameters().Select((ParameterInfo p) => p.ToString()).ToArray();
                            errors.Append(string.Format("Config file option '/{0}' has invalid parameters '({1})', maybe not compatible with '({2})'" + Environment.NewLine + "\t",
                                key,
                                value,
                                string.Join(", ", parametersName)));

                            errors.Append(e.Message + Environment.NewLine);
                        }
                    } 
                }
            }
            if (errors.Length != 0)
            { 
                throw new Error(errors.ToString());
            }
        }

        public static void ValidateParameters()
        {
            // First process the config file, then override using command line args
            ProcessConfigFile();

            if(string.IsNullOrEmpty(RepositoryPath))
            {
                // Assume the scripts are inside the repo
                RepositoryPath = Main.GetRepositoryRootPath();
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