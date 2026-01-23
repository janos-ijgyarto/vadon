using Sharpmake;
using System.IO;

[module: Sharpmake.Include("common/common.sharpmake.cs")]

namespace Vadon.Editor
{
    public abstract class Project : Engine.EngineProject
    {
        public Project()
            : base()
        {
        }

        public override void ConfigureAll(Configuration conf, Engine.Target target)
        {
            base.ConfigureAll(conf, target);
                        
            conf.ProjectPath += "/editor";
            conf.SolutionFolder = "Editor";

            conf.TargetPath = $"{BuildPath}/editor/[project.Name]/[target.Platform]/[target.Optimization]/[target.BuildSystem]";
            conf.IntermediatePath = $"{BuildPath}/editor/obj/[project.Name]/[target.Platform]/[target.Optimization]/[target.BuildSystem]";
        }

        protected static string EditorPath { get { return Path.Combine(Utilities.ConfigurationParameters.RepositoryPath, "editor"); } }
    }
}