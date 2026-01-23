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

        [Sharpmake.Configure()]
        virtual public void ConfigureAll(Configuration conf, Engine.Target target)
        {
            conf.Name = "[target.Optimization]";
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
            
            switch(target.Platform)
            {
                case Platform.win64:         
                    conf.IncludePaths.Add($"{ApplicationInstallPath}/x64-windows/include");
                    // NOTE: not adding library paths, need to let dependencies decide which optimization uses which build
                    break;
            }
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

            conf.ProjectPath += "/third_party";

            // No need to include its own path
            conf.IncludePaths.Clear();

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
    }

    [Sharpmake.Export]
    class ImGui : InstalledProject
    {
        public override void ConfigureAll(Configuration conf, Engine.Target target)
        {
            base.ConfigureAll(conf, target);

            conf.Output = Configuration.OutputType.Lib;

            conf.TargetFileName = "imgui";
            if(target.Optimization != Engine.Optimization.Debug)
            {
                conf.TargetLibraryPath = $"{ApplicationInstallPath}/x64-windows/lib";
            }
            else
            {
                conf.TargetLibraryPath = $"{ApplicationInstallPath}/x64-windows/debug/lib";
                conf.TargetFileName += "d";
            }
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
            if(target.Optimization != Engine.Optimization.Debug)
            {
                conf.TargetPath = $"{ApplicationInstallPath}/x64-windows/bin";
                conf.TargetLibraryPath = $"{ApplicationInstallPath}/x64-windows/lib";
            }
            else
            {
                conf.TargetPath = $"{ApplicationInstallPath}/x64-windows/debug/bin";
                conf.TargetLibraryPath = $"{ApplicationInstallPath}/x64-windows/debug/lib";

                conf.TargetFileName += "d";
            }
        }
    }
}