using Sharpmake;
using System.IO;

[module: Sharpmake.Include("common/common.sharpmake.cs")]
[module: Sharpmake.Include("core/core.sharpmake.cs")]
[module: Sharpmake.Include("graphics_api/graphics_api.sharpmake.cs")]
[module: Sharpmake.Include("render/render.sharpmake.cs")]
[module: Sharpmake.Include("third_party/third_party.sharpmake.cs")]

namespace Vadon.Engine
{
    [Fragment, System.Flags]
    public enum Optimization
    {
        Debug = 1 << 0,
        Dev = 1 << 1, // NOTE: equivalent to "Release with Debug Info"
        Profile = 1 << 2,
        Release = 1 << 3
    }

    // FIXME: move these to utility file!
    public enum ShaderTarget
    {
        Vertex,
        Pixel
    }

    public enum ShaderExportType
    {
        ShaderFile,
        CPP
    }

    public class Target : Sharpmake.ITarget
    {
        public DevEnv DevEnvironment;
        public Platform Platform;
        
        public Optimization Optimization;
        public BuildSystem BuildSystem;

        public static Target[] GetDefaultTargets()
        {
            return new Target[]
            {
                new Target{
                        DevEnvironment = DevEnv.vs2022,
                        Platform = Platform.win64,
                        Optimization = Optimization.Debug | Optimization.Dev | Optimization.Profile | Optimization.Release,
                        BuildSystem = BuildSystem.MSBuild | BuildSystem.FastBuild
                }
            };
        }
    }

    public abstract class Project : Sharpmake.Project
    {
        public Project()
            : base(typeof(Target))
        {
            AddTargets(Target.GetDefaultTargets());
        }

        [Sharpmake.Configure()]
        virtual public void ConfigureAll(Configuration conf, Target target)
        {
            conf.Name = "[target.Optimization]";
            conf.ProjectFileName = "[project.Name]_[target.DevEnvironment]_[target.Platform]_[target.BuildSystem]";

            switch(target.Platform)
            {
                case Platform.win64:
                    conf.Defines.Add("VADON_PLATFORM_WIN32");
                    break;
            }

            switch(target.Optimization)
            {
                case Optimization.Debug:
                    conf.Options.Add(Sharpmake.Options.Vc.Compiler.RuntimeLibrary.MultiThreadedDebugDLL);
                    conf.DefaultOption = Options.DefaultTarget.Debug;
                    break;
                default:
                    conf.Options.Add(Sharpmake.Options.Vc.Compiler.RuntimeLibrary.MultiThreadedDLL);
                    conf.DefaultOption = Options.DefaultTarget.Release;
                    break;
            }
        }

        [Configure(BuildSystem.FastBuild)]
        virtual public void ConfigureFastBuild(Configuration conf, Target target)
        {
            conf.IsFastBuild = true;
        }

        protected static string EnginePath { get { return Path.Combine(Utilities.ConfigurationParameters.RepositoryPath, "engine"); } }
        protected static string GeneratorPath { get { return Utilities.ConfigurationParameters.GeneratorOutputPath; } }
        protected static string BuildPath { get { return Utilities.ConfigurationParameters.BuildPath; } }
    }

    [Sharpmake.Generate]
    public class Foundation : Project
    {
        public Foundation()
            : base()
        {
            Name = Vadon.Foundation.FoundationBase.Name;
            SourceRootPath = Vadon.Foundation.FoundationBase.RootPath;
        }

        public override void ConfigureAll(Configuration conf, Target target)
        { 
            base.ConfigureAll(conf, target);

            conf.ProjectPath = $"{GeneratorPath}/engine/foundation";
            conf.SolutionFolder = "Foundation";

            conf.IncludePaths.Add(SourceRootPath);

            conf.Output = Configuration.OutputType.Utility;
        }
    }

    public abstract class EngineProject : Project
    {
        public EngineProject()
            : base()
        {
        }

        public override void ConfigureAll(Configuration conf, Target target)
        {
            base.ConfigureAll(conf, target);
                        
            conf.ProjectPath = $"{GeneratorPath}/engine";
            conf.SolutionFolder = "Engine";

            conf.TargetFileName = "vadon[project.Name]";
            conf.TargetPath = $"{BuildPath}/engine/[project.Name]/[target.Platform]/[target.Optimization]/[target.BuildSystem]";
            conf.IntermediatePath = $"{BuildPath}/engine/obj/[project.Name]/[target.Platform]/[target.Optimization]/[target.BuildSystem]";

            conf.IncludePaths.Add(SourceRootPath);

            conf.Options.Add(Sharpmake.Options.Vc.Compiler.CppLanguageStandard.CPP20);
            conf.Options.Add(Sharpmake.Options.Vc.Compiler.Exceptions.Enable); // FIXME: we should probably disable exceptions!
            conf.Options.Add(Sharpmake.Options.Vc.General.TreatWarningsAsErrors.Enable);
            {
                string[] warningsToDisable =
                {
                    "4251" // Type needs to have dll-interface to be used by clients
                };
                conf.Options.Add(new Sharpmake.Options.Vc.Compiler.DisableSpecificWarnings(warningsToDisable));
            }

            conf.ObjectFileName = ObjectFileName;
        }

        public override void ConfigureFastBuild(Configuration conf, Target target)
        {
            base.ConfigureFastBuild(conf, target);

            conf.FastBuildUnityPath = "[conf.IntermediatePath]/unity";

            // Force writing to pdb from different cl.exe process to go through the pdb server
            // FIXME: only do this with MSVC!
            conf.AdditionalCompilerOptions.Add("/FS");
        }

        private string ObjectFileName(string input)
        {
            // FIXME: find way to only do this for files that need it!
            return Path.Combine("[conf.IntermediatePath]/", Path.ChangeExtension(input, ".obj"));
        }

        // FIXME: find better way to make this work!
        // Use some kind of custom command to build shader compiler and access the output path via dependency
        protected static string GetShaderCompilerPath() { return $"{BuildPath}/tools/shadercompiler/[target.Platform]/release/fastbuild/shadercompiler.exe"; }
        
        protected static string GeneratedShaderFileRoot { get { return "[conf.IntermediatePath]/shaders"; } }

        protected void AddShaderCompileStep(Configuration conf, Target target, string shaderFile, ShaderTarget shaderTarget, string shaderEntryPoint, string shaderNamespace, ShaderExportType exportType = ShaderExportType.ShaderFile)
        {
            // FIXME: make this platform-agnostic!
            string fileSuffix = "";
            string targetString = ""; // FIXME: replace target string with numeric code?
            switch(shaderTarget)
            {
                case ShaderTarget.Vertex:
                    fileSuffix = "_VS";
                    targetString = "VERTEX";
                    break;
                case ShaderTarget.Pixel:
                    fileSuffix = "_PS";
                    targetString = "PIXEL";
                    break;
            }

            string exportTypeString = "";
            switch(exportType)
            {
                case ShaderExportType.ShaderFile:
                    exportTypeString = "shader";
                    break;
                case ShaderExportType.CPP:
                    exportTypeString = "cpp";
                    break;
            }

            string relativePath = Path.GetDirectoryName(Path.GetRelativePath(SourceRootPath, shaderFile));
            string outputPath = Path.Combine($"{GeneratedShaderFileRoot}/{relativePath}",$"{Path.GetFileNameWithoutExtension(shaderFile)}{fileSuffix}.hpp");
            
            // FIXME: exe args resolver uses "config" instead of "conf"
            // Report to devs and fix once patched
            string argsOutputPath = Path.Combine($"[config.IntermediatePath]/shaders/{relativePath}", $"{Path.GetFileNameWithoutExtension(shaderFile)}{fileSuffix}.hpp");

            conf.CustomFileBuildSteps.Add(
                new Configuration.CustomFileBuildStep
                {
                    KeyInput = shaderFile,
                    Output = outputPath,
                    Description = $"Generate {outputPath}",
                    Executable = GetShaderCompilerPath(),
                    ExecutableArguments = $"{SourceRootPath} {shaderFile} {argsOutputPath} {targetString} {shaderEntryPoint} {exportTypeString} {shaderNamespace}"
                }
            );
        }
    }

    public abstract class EngineLibrary : EngineProject
    {
        public EngineLibrary()
            : base()
        {
        }

        public override void ConfigureAll(Configuration conf, Target target)
        {
            base.ConfigureAll(conf, target);

            // Link statically in release
            if(target.Optimization != Optimization.Release)
            {
                conf.Output = Configuration.OutputType.Dll;
                conf.Defines.Add("VADON_LINK_DYNAMIC");
            }
            else
            {                
                conf.Output = Configuration.OutputType.Lib;
            }
        }
    }
 
    [Sharpmake.Generate]
    public class Solution : Sharpmake.Solution
    {
        public Solution()
            : base(typeof(Target))
        {
            Name = "Vadon";
 
            AddTargets(Target.GetDefaultTargets());
        }
 
        [Configure()]
        public void ConfigureAll(Configuration conf, Target target)
        {
            conf.Name = "[target.Optimization]";
            conf.SolutionFileName = "[solution.Name]_[target.DevEnvironment]_[target.Platform]_[target.BuildSystem]";
            conf.SolutionPath = $"{Utilities.ConfigurationParameters.GeneratorOutputPath}/engine";

            conf.AddProject<ThirdParty.InstallDependencies>(target);
            conf.AddProject<Core>(target);
            
            conf.AddProject<Application.ThirdParty.InstallDependencies>(target);
            conf.AddProject<Application.Application>(target);

            conf.AddProject<Demo.Editor>(target);
            conf.AddProject<Demo.Game>(target);
        }
    }
}