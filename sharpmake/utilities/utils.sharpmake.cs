using System.IO;

namespace Vadon.Utilities
{
    public static class UtilityFunctions
    {
        public static string GetThisFilePath([System.Runtime.CompilerServices.CallerFilePath] string path = null)
        {
            return path;
        }

        public static MemoryStream ConvertStringToMemoryStream(string input)
        {
            MemoryStream memoryStream = new MemoryStream();
            StreamWriter streamWriter = new StreamWriter(memoryStream);
            streamWriter.Write(input);
            streamWriter.Flush();

            return memoryStream;
        }

        public static bool FileWriteIfDifferent(string filePath, string content)
        {
            using(MemoryStream memoryStream = ConvertStringToMemoryStream(content))
            {            
                FileInfo outputFileInfo = new FileInfo(filePath);
                return Sharpmake.Util.FileWriteIfDifferent(outputFileInfo, memoryStream);
            }
        }
    }
}