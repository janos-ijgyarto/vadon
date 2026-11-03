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
            conf.SolutionFolder += "/Editor";
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
            conf.SolutionFolder += "/Editor";
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

            conf.TargetFileName = "VadonEditorCommon";
            conf.ProjectPath += "/common";

            conf.AddPublicDependency<Foundation>(target, DependencySetting.DefaultWithoutLinking);
            conf.AddPublicDependency<ThirdParty.AsioLib>(target);
        }

        [ConfigurePriority(ConfigurePriorities.Optimization)]
        [Configure(Optimization.Debug)]
        public virtual void ConfigureNonReleaseLinking(Configuration conf, Target target)
        {    
            // In all non-release builds, we create DLLs and link dynamically
            conf.Output = Configuration.OutputType.Dll;
            conf.Defines.Add("VADON_LINK_DYNAMIC");
            conf.Defines.Add("VADONEDITORCOMMON_EXPORTS");
        }

        [ConfigurePriority(ConfigurePriorities.Optimization)]
        [Configure(Optimization.Release)]
        public virtual void ConfigureReleaseLinking(Configuration conf, Target target)
        {
            // In release builds, we link everything statically         
            conf.Output = Configuration.OutputType.Lib;
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

            conf.TargetFileName = "VadonEditor";
            conf.ProjectPath += "/application";

            conf.Output = Configuration.OutputType.Exe;

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
        }

        [ConfigurePriority(ConfigurePriorities.Optimization)]
        [Configure(Optimization.Debug)]
        public virtual void ConfigureNonReleaseLinking(Configuration conf, Target target)
        {    
            // In all non-release builds, we create DLLs and link dynamically            
            conf.Defines.Add("VADON_LINK_DYNAMIC");
        }

        public override void ConfigureWin64(Configuration conf, Target target)
        {
            base.ConfigureWin64(conf, target);
                        
            string platformDLLName = target.Optimization == Optimization.Debug ? "qwindowsd" : "qwindows";
            conf.TargetCopyFilesToSubDirectory.Add(
                new System.Collections.Generic.KeyValuePair<string, string>(
                    Path.Combine(ThirdParty.QtModule.QtPath, $"plugins/platforms/{platformDLLName}.dll"), 
                    "plugins/platforms"
                )
            );
            // TODO: copy PDB as well?
        }

        public override void ConfigureFastBuild(Configuration conf, Target target)
        {
            base.ConfigureFastBuild(conf, target);

            // FIXME: Qt Moc/Uic/etc. causing issues with blobbing
            // Figure out a fix later
            conf.FastBuildBlobbed = false;
        }

        public const string EditorPluginFileExtension = "vdeplugin";

        public static string GetEditorPluginImportFilePath(Configuration conf)
        {            
            string outputPath = "[conf.IntermediatePath]/generated/editor_plugin_import/";
            string outputFileName = "[conf.TargetFileFullNameWithExtension]." + EditorPluginFileExtension;

            return Path.Combine(outputPath, outputFileName);
        }

        public static string GetResolvedEditorPluginImportFilePath(Configuration conf)
        {            
            string outputPath = $"{conf.IntermediatePath}/generated/editor_plugin_import/";
            string outputFileName = conf.TargetFileFullNameWithExtension + "." + EditorPluginFileExtension;

            return Path.Combine(outputPath, outputFileName);
        }

        public static void GenerateEditorPluginImportFile(Configuration conf)
        {
            // TODO: find some way for the resolver to generate the JSON contents?
            string configString = $"{conf.Target.Name}";
            
            string jsonString = $@"{{
    ""configuration"" : ""{configString}""
}}";
 
            Utilities.UtilityFunctions.FileWriteIfDifferent(GetResolvedEditorPluginImportFilePath(conf), jsonString);
        }

        public const string EditorGameExecutableFileExtension = "vdgexe";

        public static string GetEditorGameExecutableImportFilePath(Configuration conf)
        {            
            string outputPath = "[conf.IntermediatePath]/generated/editor_game_import/";
            string outputFileName = "[conf.TargetFileFullNameWithExtension]." + EditorGameExecutableFileExtension;

            return Path.Combine(outputPath, outputFileName);
        }

        public static string GetResolvedEditorGameExecutableImportFilePath(Configuration conf)
        {            
            string outputPath = $"{conf.IntermediatePath}/generated/editor_game_import/";
            string outputFileName = conf.TargetFileFullNameWithExtension + "." + EditorGameExecutableFileExtension;

            return Path.Combine(outputPath, outputFileName);
        }

        public static void GenerateGameExecutableImportFile(Configuration conf)
        {
            // TODO: find some way for the resolver to generate the JSON contents?
            string configString = $"{conf.Target.Name}";
            
            string jsonString = $@"{{
    ""configuration"" : ""{configString}""
}}";
 
            Utilities.UtilityFunctions.FileWriteIfDifferent(GetResolvedEditorGameExecutableImportFilePath(conf), jsonString);
        }
    }
}