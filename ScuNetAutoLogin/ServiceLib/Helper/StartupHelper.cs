using System.Runtime.Versioning;
using Microsoft.Win32;

namespace ServiceLib.Helper;

public static class StartupHelper
{
    private const string RunRegistryPath = @"Software\Microsoft\Windows\CurrentVersion\Run";
    private const string StartupValueName = "ScuNetAutoLogin";
    private const string LegacyStartupValueName = "SAL";

    public static bool AddStartup()
    {
        if (!OperatingSystem.IsWindows())
        {
            return false;
        }

        try
        {
            using var key = OpenOrCreateRunKey();

            key.SetValue(StartupValueName, BuildStartupCommand(), RegistryValueKind.String);
            return true;
        }
        catch
        {
            return false;
        }
    }

    public static bool RemoveStartup()
    {
        if (!OperatingSystem.IsWindows())
        {
            return false;
        }

        try
        {
            using var key = OpenRunKey();
            if (key is null)
            {
                return false;
            }

            key.DeleteValue(StartupValueName, false);
            key.DeleteValue(LegacyStartupValueName, false);
            return true;
        }
        catch
        {
            return false;
        }
    }

    public static bool IsStartupEnabled()
    {
        if (!OperatingSystem.IsWindows())
        {
            return false;
        }

        try
        {
            using var key = OpenRunKey(true);
            var value = key?.GetValue(StartupValueName) as string;
            return string.Equals(value, BuildStartupCommand(), StringComparison.OrdinalIgnoreCase);
        }
        catch
        {
            return false;
        }
    }

    private static string BuildStartupCommand()
    {
        var processPath = Environment.ProcessPath;
        var executablePath = string.IsNullOrWhiteSpace(processPath)
            ? Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "Sal.GUI.exe")
            : processPath;

        if (!executablePath.EndsWith("Sal.GUI.exe", StringComparison.OrdinalIgnoreCase))
        {
            executablePath =
                Path.Combine(Path.GetDirectoryName(executablePath) ?? AppDomain.CurrentDomain.BaseDirectory,
                    "Sal.GUI.exe");
        }

        return $"\"{executablePath}\" cli";
    }

    [SupportedOSPlatform("windows")]
    private static RegistryKey OpenOrCreateRunKey()
    {
        return Registry.CurrentUser.OpenSubKey(RunRegistryPath, true)
               ?? Registry.CurrentUser.CreateSubKey(RunRegistryPath, true);
    }

    [SupportedOSPlatform("windows")]
    private static RegistryKey? OpenRunKey(bool readOnly = false)
    {
        return Registry.CurrentUser.OpenSubKey(RunRegistryPath, !readOnly);
    }
}
