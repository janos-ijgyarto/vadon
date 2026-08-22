using Sharpmake;

namespace Vadon.Engine
{
    [Sharpmake.Generate]
    public class Core : EngineLibrary
    {
        public Core()
            : base()
        {
            Name = "Core";
            SourceRootPath = $"{EnginePath}/core";
        }

        public override void ConfigureAll(Configuration conf, Target target)
        {
            base.ConfigureAll(conf, target);

            conf.ProjectPath += "/core";

            conf.Defines.Add("VADONCORE_EXPORTS");
            
            conf.AddPublicDependency<GraphicsAPI>(target);
        }
    }
}