using Sharpmake;
using System.Collections.Generic;

namespace Vadon.Tools
{
    [Sharpmake.Generate]
    class ShaderCompiler : ToolsProject
    {
        private static List<KeyValuePair<ITarget, string>> _compilerExeOutputPaths = new List<KeyValuePair<ITarget, string>>();

        public static string FindCompilerExePath(Target target)
        {
            foreach(KeyValuePair<ITarget, string> entry in _compilerExeOutputPaths)
            {
                if(entry.Key.IsEqualTo(target))
                {
                    return entry.Value;
                }
            }

            return null;
        }

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

        public override void PostResolve()
        {
            base.PostResolve();

            foreach(var conf in Configurations)
            {
                string targetFullPath = conf.TargetPath + "/" + conf.TargetFileFullNameWithExtension;
                _compilerExeOutputPaths.Add(new KeyValuePair<ITarget, string>(conf.Target, targetFullPath));
            }
        }
    } 
}