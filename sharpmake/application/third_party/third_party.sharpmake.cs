using Sharpmake;
using System.IO;

namespace Vadon.Application.ThirdParty
{
    [Sharpmake.Export]
    public abstract class ExportProject : Sharpmake.Project
    {
        public ExportProject()
            : base(typeof(Engine.Target))
        {            
            AddTargets(Engine.Target.GetDefaultTargets());
        }
        
        [ConfigurePriority(Engine.ConfigurePriorities.All)]
        [Sharpmake.Configure()]
        virtual public void ConfigureAll(Configuration conf, Engine.Target target)
        {
            conf.Name = "[target.Optimization]";
        }

        [ConfigurePriority(Engine.ConfigurePriorities.Platform)]
        [Configure(Platform.win64)]
        public virtual void ConfigureWin64(Configuration conf, Engine.Target target)
        {
        }

        [ConfigurePriority(Engine.ConfigurePriorities.Optimization)]
        [Configure(Engine.Optimization.Debug)]
        public virtual void ConfigureDebug(Configuration conf, Engine.Target target)
        {
        }

        [ConfigurePriority(Engine.ConfigurePriorities.Optimization)]
        [Configure(Engine.Optimization.Dev | Engine.Optimization.Profile | Engine.Optimization.Release)]
        public virtual void ConfigureNonDebug(Configuration conf, Engine.Target target)
        {
        }
    }

    [Sharpmake.Export]
    public abstract class InstalledProject : ExportProject
    {
        public static string ApplicationInstallPath { get { return Path.Combine(Utilities.ConfigurationParameters.ThirdPartyInstallPath, "application"); } }
        
        public static string ApplicationThirdPartyPath { get { return Path.Combine(Utilities.ConfigurationParameters.RepositoryPath, "application/third_party"); } }

        public InstalledProject()
            : base()
        {
        }

        public override void ConfigureAll(Configuration conf, Engine.Target target)
        {
            base.ConfigureAll(conf, target);
        }

        public override void ConfigureWin64(Configuration conf, Engine.Target target)
        {
            // NOTE: using system paths since this is 3rd party code
            conf.IncludeSystemPaths.Add($"{ApplicationInstallPath}/x64-windows/include");
            // NOTE: not adding library paths, need to let dependencies decide which optimization uses which build
        }
    }

    [Sharpmake.Generate]
    public class InstallDependencies : Project
    {
        public static string VcpkgPath { get { return Path.Combine(Utilities.ConfigurationParameters.VcpkgPath, "vcpkg.exe"); } }

        protected static string ApplicationManifestFile { get { return "vcpkg.json"; } }

        protected static string ApplicationStatusFile { get { return "vcpkg/status"; } }

        public InstallDependencies()
            : base()
        {
            Name = "InstallApplicationDependencies";
        }

        public override void ConfigureAll(Configuration conf, Engine.Target target)
        {
            base.ConfigureAll(conf, target);

            conf.Output = Configuration.OutputType.Utility;

            conf.Options.Add(Options.Vc.General.DisableFastUpToDateCheck.Enable);

            conf.ProjectPath += "/third_party";

            // No need to include its own path
            conf.IncludeSystemPaths.Clear();

            conf.SolutionFolder += "/ThirdParty";

            conf.EventPreBuildExe.Add(
                new Configuration.BuildStepExecutable(
                    VcpkgPath,
                    $"{InstalledProject.ApplicationThirdPartyPath}/{ApplicationManifestFile}",
                    $"{InstalledProject.ApplicationInstallPath}/{ApplicationStatusFile}",
                    $"install --x-manifest-root={InstalledProject.ApplicationThirdPartyPath} --x-install-root={InstalledProject.ApplicationInstallPath}"
                )
            );
            conf.ExecuteTargetCopy = true;
        }

        public override void ConfigureFastBuild(Configuration conf, Engine.Target target)
        {
            // FIXME: this needs to be a non-FastBuild project to run the utility script
            // Should instead find a way to add as a pre-build dependency before building
            // any other node
            base.ConfigureFastBuild(conf, target);
            conf.IsFastBuild = false;
        }
    }

    [Sharpmake.Export]
    class ImGui : InstalledProject
    {
        public override void ConfigureAll(Configuration conf, Engine.Target target)
        {
            base.ConfigureAll(conf, target);

            conf.Output = Configuration.OutputType.Lib;

            conf.TargetFileName = "imgui";
        }

        public override void ConfigureDebug(Configuration conf, Engine.Target target)
        {
            base.ConfigureDebug(conf, target);
                        
            conf.TargetLibraryPath = $"{ApplicationInstallPath}/x64-windows/debug/lib";
            conf.TargetFileName += "d";
        }

        public override void ConfigureNonDebug(Configuration conf, Engine.Target target)
        {
            base.ConfigureNonDebug(conf, target);

            conf.TargetLibraryPath = $"{ApplicationInstallPath}/x64-windows/lib";
        }
    }

    [Sharpmake.Export]
    class SDL : InstalledProject
    {
        public override void ConfigureAll(Configuration conf, Engine.Target target)
        {
            base.ConfigureAll(conf, target);

            conf.Output = Configuration.OutputType.Dll;

            conf.TargetFileName = "SDL2";
        }

        public override void ConfigureDebug(Configuration conf, Engine.Target target)
        {
            base.ConfigureDebug(conf, target);

            conf.TargetPath = $"{ApplicationInstallPath}/x64-windows/debug/bin";
            conf.TargetLibraryPath = $"{ApplicationInstallPath}/x64-windows/debug/lib";

            conf.TargetFileName += "d";
        }

        public override void ConfigureNonDebug(Configuration conf, Engine.Target target)
        {
            base.ConfigureNonDebug(conf, target);
            
            conf.TargetPath = $"{ApplicationInstallPath}/x64-windows/bin";
            conf.TargetLibraryPath = $"{ApplicationInstallPath}/x64-windows/lib";
        }
    }
}