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

            conf.AddPublicDependency<Engine.Common>(target);
        }

        public override void ConfigureNonReleaseLinking(Configuration conf, Engine.Target target)
        {    
            base.ConfigureNonReleaseLinking(conf, target);

            // In all non-release builds, we create DLLs and link dynamically            
            conf.Output = Configuration.OutputType.Dll;
            conf.Defines.Add("VADONEDITOR_EXPORTS");
        }

        [ConfigurePriority(Engine.ConfigurePriorities.Optimization)]
        [Configure(Engine.Optimization.Release)]
        public virtual void ConfigureReleaseLinking(Configuration conf, Engine.Target target)
        {
            // In release builds, we link everything statically         
            conf.Output = Configuration.OutputType.Lib;
        }
    }
}