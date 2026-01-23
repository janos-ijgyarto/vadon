using Sharpmake;
using System.IO;

[module: Sharpmake.Include("editor/editor.sharpmake.cs")]
[module: Sharpmake.Include("shader_compiler/shader_compiler.sharpmake.cs")]
[module: Sharpmake.Include("third_party/third_party.sharpmake.cs")]

namespace Vadon.Tools
{
    // TODO: add profile?
    [Fragment, System.Flags]
    public enum Optimization
    {
        Debug = 1 << 0,
        Release = 1 << 1
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
                        Optimization = Optimization.Debug | Optimization.Release,
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
                    break;
            }
        }

        [Configure(BuildSystem.FastBuild)]
        virtual public void ConfigureFastBuild(Configuration conf, Target target)
        {
            conf.IsFastBuild = true;
        }

        protected static string ToolsPath { get { return Path.Combine(Utilities.ConfigurationParameters.RepositoryPath, "tools"); } }
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

            conf.ProjectPath = $"{GeneratorPath}/engine";
            conf.SolutionFolder = "Foundation";

            conf.IncludePaths.Add(SourceRootPath);

            conf.Output = Configuration.OutputType.Utility;
        }
    }

    public abstract class ToolsProject : Project
    {
        public ToolsProject()
            : base()
        {
        }

        public override void ConfigureAll(Configuration conf, Target target)
        {
            base.ConfigureAll(conf, target);
                        
            conf.ProjectPath = $"{GeneratorPath}/tools";
            conf.SolutionFolder = "Tools";

            conf.TargetPath = $"{BuildPath}/tools/[project.Name]/[target.Platform]/[target.Optimization]/[target.BuildSystem]";
            conf.IntermediatePath = $"{BuildPath}/tools/obj/[project.Name]/[target.Platform]/[target.Optimization]/[target.BuildSystem]";

            conf.IncludePaths.Add(SourceRootPath);

            conf.Options.Add(Sharpmake.Options.Vc.Compiler.CppLanguageStandard.CPP20);

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
    }

    public abstract class QtProject : ToolsProject
    {
        protected Utilities.QtSharpmakeMocTool mocTool;

        public QtProject()
            : base()
        {
            mocTool = new Utilities.QtSharpmakeMocTool();

            SourceFilesExtensions.Add(".qrc", ".ui");
        }

        public override void ConfigureAll(Configuration conf, Target target)
        { 
            base.ConfigureAll(conf, target);
            
            conf.Defines.Add("QT_SHARED");

            if (target.Optimization != Optimization.Debug)
            {
                conf.Defines.Add("QT_NO_DEBUG");
            }
        }

        protected override void ExcludeOutputFiles()
        {
            base.ExcludeOutputFiles();
            mocTool.GenerateListOfFilesToMoc(this, Path.Combine(Utilities.ConfigurationParameters.QtPath, "bin/"));
        }

        // At this point all of our includes and defines have been resolved, so now we can compute the arguments to moc.
        public override void PostLink()
        {
            mocTool.GenerateMocFileSteps(this);
            base.PostLink();
        }
    }

    [Sharpmake.Generate]
    public class Solution : Sharpmake.Solution
    {
        public Solution()
            : base(typeof(Target))
        {
            Name = "VadonTools";
 
            AddTargets(Target.GetDefaultTargets());
        }
 
        [Configure()]
        public void ConfigureAll(Configuration conf, Target target)
        {
            conf.Name = "[target.Optimization]";
            conf.SolutionFileName = "[solution.Name]_[target.DevEnvironment]_[target.Platform]_[target.BuildSystem]";
            conf.SolutionPath = $"{Utilities.ConfigurationParameters.GeneratorOutputPath}/tools";

            conf.AddProject<ShaderCompiler>(target);
            conf.AddProject<Editor.Application>(target);
            conf.AddProject<ThirdParty.InstallDependencies>(target);
        }
    }
}