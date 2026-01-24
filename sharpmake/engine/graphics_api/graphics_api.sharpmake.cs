using Sharpmake;

namespace Vadon.Engine
{
    [Sharpmake.Generate]
    public class GraphicsAPI : EngineLibrary
    {
        public GraphicsAPI()
            : base()
        {
            Name = "GraphicsAPI";
            SourceRootPath = $"{EnginePath}/graphics_api";
        }

        public override void ConfigureAll(Configuration conf, Target target)
        {
            base.ConfigureAll(conf, target);

            conf.ProjectPath += "/graphics_api";

            if(conf.Output == Configuration.OutputType.Dll)
            {
                conf.Defines.Add("VADONGRAPHICSAPI_EXPORTS");
            }

            // TODO: allow client code to override this?
            conf.Defines.Add("VADON_GRAPHICS_API_DEFAULT");

            conf.AddPublicDependency<Render>(target);

            switch(target.Platform)
            {
                case Platform.win64:
                    conf.AddPrivateDependency<ThirdParty.D3D11>(target);
                    conf.AddPrivateDependency<ThirdParty.DXGI>(target);
                    conf.AddPrivateDependency<ThirdParty.DXGUID>(target);
                    break;
            }
        }
    }
}