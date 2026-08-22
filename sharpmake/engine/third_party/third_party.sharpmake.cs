using Sharpmake;
using System;
using System.IO;

namespace Vadon.Engine.ThirdParty
{
    [Sharpmake.Export]
    public abstract class ExportProject : Sharpmake.Project
    {
        public ExportProject()
            : base(typeof(Target))
        {            
            AddTargets(Target.GetDefaultTargets());
        }

        [ConfigurePriority(ConfigurePriorities.All)]
        [Sharpmake.Configure()]
        virtual public void ConfigureAll(Configuration conf, Target target)
        {
            conf.Name = "[target.Optimization]";
        }

        [ConfigurePriority(ConfigurePriorities.Platform)]
        [Configure(Platform.win64)]
        public virtual void ConfigureWin64(Configuration conf, Target target)
        {
        }

        [ConfigurePriority(ConfigurePriorities.Optimization)]
        [Configure(Optimization.Debug)]
        public virtual void ConfigureDebug(Configuration conf, Target target)
        {
        }

        [ConfigurePriority(ConfigurePriorities.Optimization)]
        [Configure(Optimization.Dev | Optimization.Profile | Optimization.Release)]
        public virtual void ConfigureNonDebug(Configuration conf, Target target)
        {
        }
    }

    [Sharpmake.Export]
    public class D3D11 : ExportProject
    {
        public D3D11()
            : base()
        {            
            Name = "d3d11";
        }

        public override void ConfigureAll(Configuration conf, Target target)
        {
            conf.Output = Configuration.OutputType.Lib;
        }
    }

    [Sharpmake.Export]
    public class DXGI : ExportProject
    {
        public DXGI()
            : base()
        {            
            Name = "dxgi";
        }

        public override void ConfigureAll(Configuration conf, Target target)
        {
            conf.Output = Configuration.OutputType.Lib;
        }
    }

    [Sharpmake.Export]
    public class DXGUID : ExportProject
    {
        public DXGUID()
            : base()
        {            
            Name = "dxguid";
        }

        public override void ConfigureAll(Configuration conf, Target target)
        {
            conf.Output = Configuration.OutputType.Lib;
        }
    }

    [Sharpmake.Export]
    public abstract class InstalledProject : ExportProject
    {
        public static string EngineThirdPartyPath { get { return Path.Combine(Utilities.ConfigurationParameters.RepositoryPath, "engine/third_party"); } }

        public static string EngineInstallPath { get { return Path.Combine(Utilities.ConfigurationParameters.ThirdPartyInstallPath, "engine"); } }
        
        public InstalledProject()
            : base()
        {
        }

        public override void ConfigureWin64(Configuration conf, Target target)
        {
            base.ConfigureWin64(conf, target);

            conf.IncludePaths.Add($"{EngineInstallPath}/x64-windows/include");
            // NOTE: not adding library paths, need to let dependencies decide which optimization uses which build
        }
    }

    [Sharpmake.Generate]
    public class InstallDependencies : EngineProject
    {
        public static string VcpkgPath { get { return Path.Combine(Utilities.ConfigurationParameters.VcpkgPath, "vcpkg.exe"); } }

        protected static string EngineManifestFile { get { return "vcpkg.json"; } }

        protected static string EngineStatusFile { get { return "vcpkg/status"; } }

        public InstallDependencies()
            : base()
        {
            Name = "InstallEngineDependencies";
        }

        public override void ConfigureAll(Configuration conf, Target target)
        {
            base.ConfigureAll(conf, target);

            conf.Output = Configuration.OutputType.Utility;

            conf.Options.Add(Options.Vc.General.DisableFastUpToDateCheck.Enable);

            conf.ProjectPath += "/third_party";

            // No need to include its own path
            conf.IncludePaths.Clear();

            conf.SolutionFolder += "/ThirdParty";

            conf.EventPreBuildExe.Add(
                new Configuration.BuildStepExecutable(
                    VcpkgPath,
                    $"{InstalledProject.EngineThirdPartyPath}/{EngineManifestFile}",
                    $"{InstalledProject.EngineInstallPath}/{EngineStatusFile}",
                    $"install --x-manifest-root={InstalledProject.EngineThirdPartyPath} --x-install-root={InstalledProject.EngineInstallPath}"
                )
            );
            conf.ExecuteTargetCopy = true;
        }
    }

    [Sharpmake.Export]
    public class ConcurrentQueue : InstalledProject
    {
        public ConcurrentQueue()
        {
            Name = "ConcurrentQueue";
        }
    }

    [Sharpmake.Export]
    public class GLM : InstalledProject
    {
        public GLM()
        {
            Name = "GLM";
        }

        public override void ConfigureAll(Configuration conf, Target target)
        {
            conf.ExportDefines.Add("GLM_ENABLE_EXPERIMENTAL");     
        }

        public override void ConfigureDebug(Configuration conf, Target target)
        {
            base.ConfigureDebug(conf, target);
                        
            conf.LibraryFiles.Add($"{EngineInstallPath}/x64-windows/debug/lib/glm.lib");
        }

        public override void ConfigureNonDebug(Configuration conf, Target target)
        {
            base.ConfigureNonDebug(conf, target);

            conf.LibraryFiles.Add($"{EngineInstallPath}/x64-windows/lib/glm.lib");
        }
    }

    [Sharpmake.Export]
    public class FreeType : InstalledProject
    {
        public FreeType()
        {
            Name = "FreeType";
        }

        public override void ConfigureAll(Configuration conf, Target target)
        {
            conf.Output = Configuration.OutputType.Dll;

            conf.TargetFileName = "freetype";
        }

        public override void ConfigureDebug(Configuration conf, Target target)
        {
            base.ConfigureDebug(conf, target);

            conf.TargetPath = $"{EngineInstallPath}/x64-windows/debug/bin";
            conf.TargetLibraryPath = $"{EngineInstallPath}/x64-windows/debug/lib";

            conf.TargetFileName += "d";
        }

        public override void ConfigureNonDebug(Configuration conf, Target target)
        {
            base.ConfigureNonDebug(conf, target);
                                    
            conf.TargetPath = $"{EngineInstallPath}/x64-windows/bin";
            conf.TargetLibraryPath = $"{EngineInstallPath}/x64-windows/lib";
        }
    }
    
    [Sharpmake.Export]
    public class NlohmannJson : InstalledProject
    {
        public NlohmannJson()
        {
            Name = "NlohmannJson";
        }
    }

    [Sharpmake.Export]
    public class SIMDJson : InstalledProject
    {
        public SIMDJson()
        {
            Name = "simdjson";
        }

        public override void ConfigureAll(Configuration conf, Target target)
        {
            conf.Output = Configuration.OutputType.Dll;

            conf.ExportDefines.Add("SIMDJSON_USING_WINDOWS_DYNAMIC_LIBRARY");

            conf.TargetFileName = "simdjson";
        }

        public override void ConfigureDebug(Configuration conf, Target target)
        {
            base.ConfigureDebug(conf, target);
                        
            conf.TargetPath = $"{EngineInstallPath}/x64-windows/debug/bin";
            conf.TargetLibraryPath = $"{EngineInstallPath}/x64-windows/debug/lib";
        }

        public override void ConfigureNonDebug(Configuration conf, Target target)
        {
            base.ConfigureNonDebug(conf, target);
            
            conf.TargetPath = $"{EngineInstallPath}/x64-windows/bin";
            conf.TargetLibraryPath = $"{EngineInstallPath}/x64-windows/lib";
        }
    }
}