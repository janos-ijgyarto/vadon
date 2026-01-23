using Sharpmake;

namespace Vadon.Tools
{
    [Sharpmake.Generate]
    class ShaderCompiler : ToolsProject
    {
        public ShaderCompiler()
            : base()
        {
            Name = "ShaderCompiler";
            SourceRootPath = $"{ToolsPath}/shader_compiler";
        }

        public override void ConfigureAll(Configuration conf, Target target)
        {
            base.ConfigureAll(conf, target);

            conf.ProjectPath += "/shader_compiler";

            conf.Output = Configuration.OutputType.Exe;

            switch(target.Platform)
            {
                case Platform.win64:
                    conf.AddPrivateDependency<ThirdParty.D3DCompiler>(target);
                    conf.Defines.Add("VADON_D3D_COMPILER");
                    break;
            }
        } 
    } 
}