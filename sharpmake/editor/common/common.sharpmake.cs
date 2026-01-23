using Sharpmake;

namespace Vadon.Editor
{
    [Sharpmake.Generate]
    public class Common : Project
    {
        public Common()
            : base()
        {
            Name = "EditorPluginCommon";
            SourceRootPath = $"{EditorPath}/common";
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
                conf.Defines.Add("VADONEDITOR_EXPORTS");
            }
            else
            {                
                conf.Output = Configuration.OutputType.Lib;
            }

            conf.AddPublicDependency<Engine.Common>(target);
        }
    }
}