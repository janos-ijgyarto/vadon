using Sharpmake;
using System.IO;

namespace Vadon.Engine
{
    [Sharpmake.Generate]
    public class Render : EngineLibrary
    {
        public Render()
            : base()
        {
            Name = "Render";
            SourceRootPath = $"{EnginePath}/render";

            SourceFilesExtensions.Add(".hlsl");
        }

        public override void ConfigureAll(Configuration conf, Target target)
        {
            base.ConfigureAll(conf, target);

            conf.ProjectPath += "/render";

            if(conf.Output == Configuration.OutputType.Dll)
            {
                conf.Defines.Add("VADONRENDER_EXPORTS");
            }

            // Add path to generated shaders
            conf.IncludePrivatePaths.Add(GeneratedShaderFileRoot);
            
            conf.AddPublicDependency<Common>(target);
            conf.AddPublicDependency<ThirdParty.FreeType>(target);

            AddShaderCompileStep(conf, target, $"{SourceRootPath}/Vadon/Private/Render/Canvas/Shader.hlsl", ShaderTarget.Vertex, "vs_main", "Vadon::Private::Render::Canvas::ShaderVS", ShaderExportType.CPP);
            AddShaderCompileStep(conf, target, $"{SourceRootPath}/Vadon/Private/Render/Canvas/Shader.hlsl", ShaderTarget.Pixel, "ps_main", "Vadon::Private::Render::Canvas::ShaderPS", ShaderExportType.CPP);
        }
    }
}