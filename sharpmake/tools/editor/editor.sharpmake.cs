using Sharpmake;
using System.IO;

namespace Vadon.Tools.Editor
{
    public class Project : ToolsProject
    {
        public Project()
            : base()
        {

        }

        public override void ConfigureAll(Configuration conf, Target target)
        { 
            base.ConfigureAll(conf, target);

            conf.ProjectPath += "/editor";
            conf.SolutionFolder = "Editor";
        }

        protected static string EditorPath { get { return $"{ToolsPath}/editor"; } }
    }

    public class QtProject : Tools.QtProject
    {
        public QtProject()
            : base()
        {

        }

        public override void ConfigureAll(Configuration conf, Target target)
        { 
            base.ConfigureAll(conf, target);

            conf.ProjectPath += "/editor";
            conf.SolutionFolder = "Editor";
        }

        protected static string EditorPath { get { return $"{ToolsPath}/editor"; } }
    }

    [Sharpmake.Generate]
    public class Common : Project
    {
        public Common()
            : base()
        {
            Name = "EditorCommon";
            SourceRootPath = $"{EditorPath}/common";
        }

        public override void ConfigureAll(Configuration conf, Target target)
        { 
            base.ConfigureAll(conf, target);

            conf.ProjectPath += "/common";

            if(target.Optimization != Optimization.Release)
            {
                conf.Output = Configuration.OutputType.Dll;
                conf.Defines.Add("VADON_LINK_DYNAMIC");
                conf.Defines.Add("VADONEDITORCOMMON_EXPORTS");
            }
            else
            {                
                conf.Output = Configuration.OutputType.Lib;
            }

            conf.AddPublicDependency<Foundation>(target, DependencySetting.DefaultWithoutLinking);
            conf.AddPublicDependency<ThirdParty.AsioLib>(target);
        }
    }

    [Sharpmake.Generate]
    public class Application : QtProject
    {
        public Application()
            : base()
        {
            Name = "EditorApplication";
            SourceRootPath = $"{EditorPath}/application";
        }

        public override void ConfigureAll(Configuration conf, Target target)
        { 
            base.ConfigureAll(conf, target);

            conf.ProjectPath += "/application";

            conf.Output = Configuration.OutputType.Exe;

            if(target.Optimization != Optimization.Release)
            {
                conf.Defines.Add("VADON_LINK_DYNAMIC");
            }

            conf.AddPrivateDependency<Common>(target);

            // FIXME: need to add this explicitly because for some reason Common did not propagate
            // via the public dependency
            // This might be because Sharpmake cannot "know" that the editor also uses code from Asio directly
            // so the dependency only ensures certain levels of "compatibility"
            conf.AddPrivateDependency<ThirdParty.AsioLib>(target);

            conf.AddPrivateDependency<ThirdParty.QtCore>(target);
            conf.AddPrivateDependency<ThirdParty.QtGui>(target);
            conf.AddPrivateDependency<ThirdParty.QtWidgets>(target);

            // FIXME: have to enable RTTI to make Qt work
            // Need to compile Qt with RTTI turned off!
            conf.Options.Add(Sharpmake.Options.Vc.Compiler.RTTI.Enable);

            switch(target.Platform)
            {
                case Platform.win64:
                    string platformDLLName = target.Optimization == Optimization.Debug ? "qwindowsd" : "qwindows";
                    conf.TargetCopyFilesToSubDirectory.Add(
                        new System.Collections.Generic.KeyValuePair<string, string>(
                            Path.Combine(ThirdParty.QtModule.QtPath, $"plugins/platforms/{platformDLLName}.dll"), 
                            "plugins/platforms"
                        )
                    );
                    // TODO: copy PDB as well?
                    break;
            }
        }

        public override void ConfigureFastBuild(Configuration conf, Target target)
        {
            base.ConfigureFastBuild(conf, target);

            // FIXME: Qt Moc/Uic/etc. causing issues with blobbing
            // Figure out a fix later
            conf.FastBuildBlobbed = false;
        }
    }
}