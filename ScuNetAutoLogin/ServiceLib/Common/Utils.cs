namespace ServiceLib.Common;

public class Utils
{
    public static string GetBaseDirectory(string fileName = "")
    {
        return Path.Combine(AppDomain.CurrentDomain.BaseDirectory, fileName);
    }

    public static string GetConfigPath()
    {
        return GetBaseDirectory("config");
    }
}