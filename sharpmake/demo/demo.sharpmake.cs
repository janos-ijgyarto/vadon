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
            
            conf.AddPublicDependency<Engine.Render>(target);
        }

        [ConfigurePriority(Engine.ConfigurePriorities.Optimization)]
        [Configure(Engine.Optimization.Debug | Engine.Optimization.Dev | Engine.Optimization.Profile)]
        public virtual void ConfigureNonReleaseLinking(Configuration conf, Engine.Target target)
        {    
            // In all non-release builds, we create DLLs and link dynamically            
            conf.Output = Configuration.OutputType.Dll;
            conf.Defines.Add("VADON_LINK_DYNAMIC");
            conf.Defines.Add("VADONDEMO_EXPORTS");
        }

        [ConfigurePriority(Engine.ConfigurePriorities.Optimization)]
        [Configure(Engine.Optimization.Release)]
        public virtual void ConfigureReleaseLinking(Configuration conf, Engine.Target target)
        {
            // In release builds, we link everything statically         
            conf.Output = Configuration.OutputType.Lib;
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

            // Adding macro to export the plugin functions
            conf.Defines.Add("VADON_EDITOR_ASSET_SERVER_PLUGIN_IMPLEMENTATION");
            conf.Defines.Add("VADON_EDITOR_SIMULATOR_PLUGIN_IMPLEMENTATION");

            conf.AddPrivateDependency<Common>(target);
            conf.AddPrivateDependency<Engine.Common>(target);
            conf.AddPrivateDependency<Engine.Render>(target);
            conf.AddPrivateDependency<Engine.Core>(target);
            conf.AddPrivateDependency<Vadon.Editor.Common>(target);

            // Copy the plugin import file
            // FIXME: some way to do this in one step, to ensure that we don't miss it here or in PostResolve?
            conf.TargetCopyFiles.Add(Tools.Editor.Application.GetEditorPluginImportFilePath(conf));

            // NOTE: forcing the files to be copied because the editor needs to load them
            // from the same subdirectory
            conf.ExecuteTargetCopy = true;
        }

        [ConfigurePriority(Engine.ConfigurePriorities.Optimization)]
        [Configure(Engine.Optimization.Debug | Engine.Optimization.Dev | Engine.Optimization.Profile)]
        public virtual void ConfigureNonReleaseLinking(Configuration conf, Engine.Target target)
        {    
            // Link dynamically to engine libraries
            conf.Defines.Add("VADON_LINK_DYNAMIC");
        }

        public override void PostResolve()
        {
            base.PostResolve();

            foreach(var currentConf in Configurations)
            {                
                Tools.Editor.Application.GenerateEditorPluginImportFile(currentConf);
            }
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

            // Add path to generated shaders
            conf.IncludePrivatePaths.Add(GeneratedShaderFileRoot);

            conf.AddPrivateDependency<Common>(target);
            conf.AddPrivateDependency<Engine.Common>(target);
            conf.AddPrivateDependency<Engine.Core>(target);
            conf.AddPrivateDependency<Application.Application>(target);

            AddShaderCompileStep(conf, target, $"{SourceRootPath}/VadonDemo/Render/CopyShader.hlsl", Engine.ShaderTarget.Vertex, "vs_main", "VadonDemo::Render::ShaderVS", Engine.ShaderExportType.CPP);
            AddShaderCompileStep(conf, target, $"{SourceRootPath}/VadonDemo/Render/CopyShader.hlsl", Engine.ShaderTarget.Pixel, "ps_main", "VadonDemo::Render::ShaderPS", Engine.ShaderExportType.CPP);
                   
            // Copy the game exe import file
            // FIXME: some way to do this in one step, to ensure that we don't miss it here or in PostResolve?
            conf.TargetCopyFiles.Add(Tools.Editor.Application.GetEditorGameExecutableImportFilePath(conf));
        }

        [ConfigurePriority(Engine.ConfigurePriorities.Optimization)]
        [Configure(Engine.Optimization.Debug | Engine.Optimization.Dev | Engine.Optimization.Profile)]
        public virtual void ConfigureNonReleaseLinking(Configuration conf, Engine.Target target)
        {    
            // Link dynamically to engine libraries
            conf.Defines.Add("VADON_LINK_DYNAMIC");
        }

        public override void PostResolve()
        {
            base.PostResolve();

            foreach(var currentConf in Configurations)
            {
                Tools.Editor.Application.GenerateGameExecutableImportFile(currentConf);
            }
        }
    }
}