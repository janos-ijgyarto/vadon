using Sharpmake;
using System.IO;

namespace Vadon.Foundation
{
    // TODO: find way to create actual base class?
    // Engine and tools might have different targets!
    public static class FoundationBase
    {
        public static string Name = "Foundation";
        public static string FolderName = "Foundation";
        public static string RootPath { get { return Path.Combine(Utilities.ConfigurationParameters.RepositoryPath, "foundation"); } }
    }
}