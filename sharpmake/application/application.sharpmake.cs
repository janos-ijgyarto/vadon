using Sharpmake;
using System.IO;

[module: Sharpmake.Include("third_party/third_party.sharpmake.cs")]

namespace Vadon.Application
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
                        
            conf.ProjectPath += "/application";
            conf.SolutionFolder = "Application";

            conf.TargetPath = $"{BuildPath}/application/[project.Name]/[target.Platform]/[target.Optimization]/[target.BuildSystem]";
            conf.IntermediatePath = $"{BuildPath}/application/obj/[project.Name]/[target.Platform]/[target.Optimization]/[target.BuildSystem]";
        }

        protected static string ApplicationPath { get { return Path.Combine(Utilities.ConfigurationParameters.RepositoryPath, "application"); } }
    }

    [Sharpmake.Generate]
    public class Application : Project
    {
        public Application()
            : base()
        {
            Name = "Application";
            SourceRootPath = ApplicationPath;

            SourceFilesExtensions.Add(".hlsl");
        }

        public override void ConfigureAll(Configuration conf, Engine.Target target)
        { 
            base.ConfigureAll(conf, target);

            if(target.Optimization != Engine.Optimization.Release)
            {
                conf.Output = Configuration.OutputType.Dll;
                conf.Defines.Add("VADON_LINK_DYNAMIC");
                conf.Defines.Add("VADONAPP_EXPORTS");
            }
            else
            {                
                conf.Output = Configuration.OutputType.Lib;
            }

            // Add path to generated shaders
            conf.IncludePrivatePaths.Add(GeneratedShaderFileRoot);
            
            conf.AddPublicDependency<Engine.Core>(target);

            conf.AddPublicDependency<ThirdParty.SDL>(target);
            conf.Defines.Add("VADON_PLATFORM_INTERFACE_SDL");

            conf.AddPublicDependency<ThirdParty.ImGui>(target);
            conf.Defines.Add("VADON_DEVELOPER_GUI_IMGUI");

            AddShaderCompileStep(conf, target, $"{SourceRootPath}/VadonApp/Private/UI/Developer/ImGui/GUIShader.hlsl", Engine.ShaderTarget.Vertex, "vs_main", "VadonApp::Private::UI::Developer::ImGUI::ShaderVS", Engine.ShaderExportType.CPP);
            AddShaderCompileStep(conf, target, $"{SourceRootPath}/VadonApp/Private/UI/Developer/ImGui/GUIShader.hlsl", Engine.ShaderTarget.Pixel, "ps_main", "VadonApp::Private::UI::Developer::ImGUI::ShaderPS", Engine.ShaderExportType.CPP);
        }
    }
}