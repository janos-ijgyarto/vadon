using Sharpmake;
using System.IO;

namespace Vadon.Tools.ThirdParty
{
    [Sharpmake.Export]
    public abstract class ExportProject : Sharpmake.Project
    {
        public ExportProject()
            : base(typeof(Target))
        {            
            AddTargets(Target.GetDefaultTargets());
        }

        [Sharpmake.Configure()]
        virtual public void ConfigureAll(Configuration conf, Target target)
        {
            conf.Name = "[target.Optimization]";
        }
    }

    [Sharpmake.Export]
    public class D3DCompiler : ExportProject
    {
        public D3DCompiler()
            : base()
        {            
            Name = "d3dcompiler";
        }

        public override void ConfigureAll(Configuration conf, Target target)
        {
            conf.Output = Configuration.OutputType.Lib;
        }
    }

    [Sharpmake.Export]
    public abstract class QtModule : ExportProject
    {
        protected string ModuleName;

        public QtModule(string moduleName)
            : base()
        {
            ModuleName = moduleName; 
        }

        public static string QtPath { get { return Utilities.ConfigurationParameters.QtPath; } }

        public override void ConfigureAll(Configuration conf, Target target)
        {
            base.ConfigureAll(conf, target);

            conf.Output = Configuration.OutputType.Dll;
                       
            conf.IncludePaths.Add(Path.Combine(QtPath, $"include"));
            conf.IncludePaths.Add(Path.Combine(QtPath, $"include/Qt{ModuleName}"));

            conf.TargetFileName = $"Qt6{ModuleName}";
            if(target.Optimization == Optimization.Debug)
            {
                conf.TargetFileName += "d";
            }

            conf.TargetPath = Path.Combine(QtPath, "bin");
            conf.TargetLibraryPath = Path.Combine(QtPath, "lib");
        }
    }

    [Sharpmake.Export]
    public class QtCore : QtModule
    {
        public QtCore()
            : base("Core")
        {
        }
    }

    [Sharpmake.Export]
    public class QtGui : QtModule
    {
        public QtGui()
            : base("Gui")
        {
        }
    }

    [Sharpmake.Export]
    public class QtWidgets : QtModule
    {
        public QtWidgets()
            : base("Widgets")
        {
        }
    }

    [Sharpmake.Export]
    public abstract class InstalledProject : ExportProject
    {
        public static string ToolsThirdPartyPath { get { return Path.Combine(Utilities.ConfigurationParameters.RepositoryPath, "tools/third_party"); } }

        public static string ToolsInstallPath { get { return Path.Combine(Utilities.ConfigurationParameters.ThirdPartyInstallPath, "tools"); } }

        public InstalledProject()
            : base()
        {
        }

        public override void ConfigureAll(Configuration conf, Target target)
        {
            base.ConfigureAll(conf, target);
            
            switch(target.Platform)
            {
                case Platform.win64:         
                    conf.IncludePaths.Add($"{ToolsInstallPath}/x64-windows/include");
                    // NOTE: not adding library paths, need to let dependencies decide which optimization uses which build
                    break;
            }
        }
    }

    [Sharpmake.Generate]
    public class InstallDependencies : ToolsProject
    {
        public static string VcpkgPath { get { return Path.Combine(Utilities.ConfigurationParameters.VcpkgPath, "vcpkg.exe"); } }
        
        protected static string ToolsManifestFile { get { return "vcpkg.json"; } }

        protected static string ToolsStatusFile { get { return "vcpkg/status"; } }
        
        public InstallDependencies()
            : base()
        {
            Name = "InstallDependencies";
        }

        public override void ConfigureAll(Configuration conf, Target target)
        {
            base.ConfigureAll(conf, target);

            conf.Output = Configuration.OutputType.Utility;

            conf.Options.Add(Options.Vc.General.DisableFastUpToDateCheck.Enable);

            conf.ProjectPath += "/third_party";

            // No need to include its own path
            conf.IncludePaths.Clear();

            conf.SolutionFolder = "ThirdParty";

            conf.EventPreBuildExe.Add(
                new Configuration.BuildStepExecutable(
                    VcpkgPath,
                    $"{InstalledProject.ToolsThirdPartyPath}/{ToolsManifestFile}",
                    $"{InstalledProject.ToolsInstallPath}/{ToolsStatusFile}",
                    $"install --x-manifest-root={InstalledProject.ToolsThirdPartyPath} --x-install-root={InstalledProject.ToolsInstallPath}"
                )
            );
            conf.ExecuteTargetCopy = true;
        }
    }

    [Sharpmake.Export]
    public class Asio : InstalledProject
    {
        public Asio()
            : base()
        {
            Name = "Asio";
        }
    }

    [Sharpmake.Generate]
    public class AsioLib : ToolsProject
    {
        public AsioLib()
            : base()
        {
            Name = "AsioLib";
            SourceRootPath = $"{InstalledProject.ToolsThirdPartyPath}/asio";
        }

        public override void ConfigureAll(Configuration conf, Target target)
        {
            base.ConfigureAll(conf, target);

            conf.ProjectPath += "/third_party";
            conf.SolutionFolder = "ThirdParty";

            // No need to include its own path
            conf.IncludePaths.Clear();

            conf.AddPublicDependency<Asio>(target);

            // Link statically in release
            if(target.Optimization != Optimization.Release)
            {
                conf.Output = Configuration.OutputType.Dll;
                
                conf.Defines.Add("ASIO_DYN_LINK");
                conf.ExportDefines.Add("ASIO_DYN_LINK");
            }
            else
            {                
                conf.Output = Configuration.OutputType.Lib;

                conf.Defines.Add("ASIO_SEPARATE_COMPILATION");
                conf.ExportDefines.Add("ASIO_SEPARATE_COMPILATION");
            }
        }
    }
}