namespace Vadon.Utilities
{
    public static class UtilityFunctions
    {
        public static string GetThisFilePath([System.Runtime.CompilerServices.CallerFilePath] string path = null)
        {
            return path;
        }
    }
}