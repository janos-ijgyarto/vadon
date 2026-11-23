using Sharpmake;

namespace Vadon.Engine
{
    [Sharpmake.Generate]
    public class RHI : EngineLibrary
    {
        public RHI()
            : base()
        {
            Name = "RHI";
            SourceRootPath = $"{EnginePath}/rhi";
        }

        public override void ConfigureAll(Configuration conf, Target target)
        {
            base.ConfigureAll(conf, target);

            conf.ProjectPath += "/rhi";

            conf.Defines.Add("VADONRHI_EXPORTS");

            // TODO: allow client code to override this?
            conf.Defines.Add("VADON_RHI_DEFAULT");

            conf.AddPublicDependency<Render>(target);
        }

        public override void ConfigureWin64(Configuration conf, Target target)
        {
            base.ConfigureWin64(conf, target);

            conf.AddPrivateDependency<ThirdParty.NRI_D3D12>(target);
        }
    }
}