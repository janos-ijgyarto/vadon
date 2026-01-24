using Sharpmake;
using System.IO;

namespace Vadon.Demo
{
    public abstract class Project : Engine.EngineProject
    {
        public Project()
            : base()
        {
        }

        public override void ConfigureAll(Configuration conf, Engine.Target target)
        {
            base.ConfigureAll(conf, target);
                        
            conf.ProjectPath += "/demo";
            conf.SolutionFolder = "Demo";

            conf.TargetPath = $"{BuildPath}/demo/[project.Name]/[target.Platform]/[target.Optimization]/[target.BuildSystem]";
            conf.IntermediatePath = $"{BuildPath}/demo/obj/[project.Name]/[target.Platform]/[target.Optimization]/[target.BuildSystem]";
        }

        protected static string DemoPath { get { return Path.Combine(Utilities.ConfigurationParameters.RepositoryPath, "demo"); } }
    }

    [Sharpmake.Generate]
    public class Common : Project
    {
        public Common()
            : base()
        {
            Name = "DemoCommon";
            SourceRootPath = $"{DemoPath}/common";
        }

        public override void ConfigureAll(Configuration conf, Engine.Target target)
        { 
            base.ConfigureAll(conf, target);

            conf.ProjectPath += "/common";
            
            // Link statically in release
            if(target.Optimization != Engine.Optimization.Release)
            {
                conf.Output = Configuration.OutputType.Dll;
                conf.Defines.Add("VADON_LINK_DYNAMIC");
                conf.Defines.Add("VADONDEMO_EXPORTS");
            }
            else
            {                
                conf.Output = Configuration.OutputType.Lib;
            }

            conf.AddPublicDependency<Engine.Render>(target);
        }
    }

    [Sharpmake.Generate]
    public class Editor : Project
    {
        public Editor()
            : base()
        {
            Name = "DemoEditor";
            SourceRootPath = $"{DemoPath}/editor";
        }

        public override void ConfigureAll(Configuration conf, Engine.Target target)
        {
            base.ConfigureAll(conf, target);

            conf.ProjectPath += "/editor";

            conf.Output = Configuration.OutputType.Dll;

            if(target.Optimization != Engine.Optimization.Release)
            {
                conf.Defines.Add("VADON_LINK_DYNAMIC");
            }

            // Adding macro to export the plugin functions
            conf.Defines.Add("VADON_EDITOR_SIMULATOR_PLUGIN_IMPLEMENTATION");

            conf.AddPrivateDependency<Common>(target);
            conf.AddPrivateDependency<Engine.Common>(target);
            conf.AddPrivateDependency<Engine.Render>(target);
            conf.AddPrivateDependency<Engine.Core>(target);
            conf.AddPrivateDependency<Vadon.Editor.Common>(target);

            // NOTE: forcing the files to be copied because the editor needs to load them
            // from the same subdirectory
            conf.ExecuteTargetCopy = true;
        }
    }

    [Sharpmake.Generate]
    public class Game : Project
    {
        public Game()
            : base()
        {
            Name = "DemoGame";
            SourceRootPath = $"{DemoPath}/game";

            SourceFilesExtensions.Add(".hlsl");
        }

        public override void ConfigureAll(Configuration conf, Engine.Target target)
        { 
            base.ConfigureAll(conf, target);

            conf.ProjectPath += "/game";

            conf.Output = Configuration.OutputType.Exe;

            if(target.Optimization != Engine.Optimization.Release)
            {
                conf.Defines.Add("VADON_LINK_DYNAMIC");
            }

            // Add path to generated shaders
            conf.IncludePrivatePaths.Add(GeneratedShaderFileRoot);

            conf.AddPrivateDependency<Common>(target);
            conf.AddPrivateDependency<Engine.Common>(target);
            conf.AddPrivateDependency<Engine.Core>(target);
            conf.AddPrivateDependency<Application.Application>(target);

            AddShaderCompileStep(conf, target, $"{SourceRootPath}/VadonDemo/Render/CopyShader.hlsl", Engine.ShaderTarget.Vertex, "vs_main", "VadonDemo::Render::ShaderVS", Engine.ShaderExportType.CPP);
            AddShaderCompileStep(conf, target, $"{SourceRootPath}/VadonDemo/Render/CopyShader.hlsl", Engine.ShaderTarget.Pixel, "ps_main", "VadonDemo::Render::ShaderPS", Engine.ShaderExportType.CPP);
        }
    }
}