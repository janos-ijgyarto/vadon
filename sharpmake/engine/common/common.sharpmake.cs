using Sharpmake;

namespace Vadon.Engine
{
    [Sharpmake.Generate]
    public class Common : EngineLibrary
    {
        public Common()
            : base()
        {
            Name = "Common";
            SourceRootPath = $"{EnginePath}/common";
        }

        public override void ConfigureAll(Configuration conf, Target target)
        {
            base.ConfigureAll(conf, target);
            
            conf.ProjectPath += "/common";

            if(conf.Output == Configuration.OutputType.Dll)
            {
                conf.Defines.Add("VADONCOMMON_EXPORTS");
            }
            
            conf.AddPublicDependency<Foundation>(target, DependencySetting.DefaultWithoutLinking);

            conf.AddPublicDependency<ThirdParty.ConcurrentQueue>(target);
            conf.AddPublicDependency<ThirdParty.GLM>(target);
            conf.AddPublicDependency<ThirdParty.NlohmannJson>(target);
        }
    }
}