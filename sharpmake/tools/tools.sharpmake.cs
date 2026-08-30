using Sharpmake;
using System.IO;

[module: Sharpmake.Include("editor/editor.sharpmake.cs")]
[module: Sharpmake.Include("shader_compiler/shader_compiler.sharpmake.cs")]
[module: Sharpmake.Include("third_party/third_party.sharpmake.cs")]

namespace Vadon.Tools
{
    public static class ConfigurePriorities
    {
        public const int All = -75;
        public const int Platform = -50;
        public const int Optimization = -25;
        /*     SHARPMAKE DEFAULT IS 0     */
        public const int BuildSystem = 50;
    }

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

        [Sharpmake.ConfigurePriority(ConfigurePriorities.All)]
        [Sharpmake.Configure()]
        virtual public void ConfigureAll(Configuration conf, Target target)
        {
            conf.Name = "[target.Optimization]";
            conf.ProjectFileName = "[project.Name]_[target.DevEnvironment]_[target.Platform]_[target.BuildSystem]";
            conf.SolutionFolder = "Vadon";
        }

        #region Platforms
        [ConfigurePriority(ConfigurePriorities.Platform)]
        [Configure(Platform.win64)]
        public virtual void ConfigureWin64(Configuration conf, Target target)
        {
            conf.Defines.Add("VADON_PLATFORM_WIN32");
            conf.Options.Add(Options.Vc.General.PlatformToolset.v143);
            conf.Options.Add(Options.Vc.General.WindowsTargetPlatformVersion.v10_0_26100_0);
        }
        #endregion

        #region Optimizations
        [ConfigurePriority(ConfigurePriorities.Optimization)]
        [Configure(Optimization.Debug)]
        public virtual void ConfigureDebug(Configuration conf, Target target)
        {
            conf.Options.Add(Sharpmake.Options.Vc.Compiler.RuntimeLibrary.MultiThreadedDebugDLL);
            conf.DefaultOption = Options.DefaultTarget.Debug;
        }

        [ConfigurePriority(ConfigurePriorities.Optimization)]
        [Configure(Optimization.Release)]
        public virtual void ConfigureRelease(Configuration conf, Target target)
        {
            conf.Options.Add(Sharpmake.Options.Vc.Compiler.RuntimeLibrary.MultiThreadedDLL);
            conf.DefaultOption = Options.DefaultTarget.Release;
        }
        #endregion

        #region Build system
        [ConfigurePriority(ConfigurePriorities.BuildSystem)]
        [Configure(BuildSystem.FastBuild)]
        virtual public void ConfigureFastBuild(Configuration conf, Target target)
        {
            conf.IsFastBuild = true;
        }
        #endregion

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
            conf.SolutionFolder = "Vadon/Foundation";

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
            conf.SolutionFolder += "/Tools";

            conf.TargetPath = $"{BuildPath}/tools/[project.Name]/[target.Platform]/[target.Optimization]/[target.BuildSystem]";
            conf.IntermediatePath = $"{BuildPath}/tools/obj/[project.Name]/[target.Platform]/[target.Optimization]/[target.BuildSystem]";

            conf.IncludePaths.Add(SourceRootPath);

            conf.Options.Add(Sharpmake.Options.Vc.Compiler.CppLanguageStandard.CPP20);
            conf.Options.Add(Sharpmake.Options.Vc.General.TreatWarningsAsErrors.Enable);
            conf.Options.Add(Sharpmake.Options.Vc.Compiler.Exceptions.Enable);

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
        }

        public override void ConfigureRelease(Configuration conf, Target target)
        {
            base.ConfigureRelease(conf, target);
            
            // Turn off asserts and debug output outside of debug builds
            conf.Defines.Add("QT_NO_DEBUG");
            conf.Defines.Add("QT_NO_DEBUG_OUTPUT");
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