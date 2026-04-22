using System.Diagnostics;
using ServiceLib.Common;

namespace ServiceLib.Helper;

public class PlatformHelper
{
    public static async Task<bool> OpenHotspots()
    {
        if (!Utils.IsWindows()) return false;
        var script = EmbedUtils.GetEmbedText(Global.HotspotScriptResourceName);
        using var process = new Process();
        process.StartInfo = new ProcessStartInfo
        {
            FileName = "powershell",
            ArgumentList = { "-NoProfile", "-ExecutionPolicy", "Bypass", "-Command", script },
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            UseShellExecute = false,
            CreateNoWindow = true
        };
        process.Start();
        var stdOutTask = process.StandardOutput.ReadToEndAsync();
        var stdErrTask = process.StandardError.ReadToEndAsync();
        using var cts = new CancellationTokenSource(8000);
        await process.WaitForExitAsync(cts.Token);
        try
        {
            await process.WaitForExitAsync(cts.Token);

            var stdout = await stdOutTask;
            var stderr = await stdErrTask;

            var output = stderr + stdout;

            return output.Contains("successfully turned on", StringComparison.OrdinalIgnoreCase)
                   || output.Contains("is already on", StringComparison.OrdinalIgnoreCase);
        }
        catch (OperationCanceledException)
        {
            try
            {
                if (!process.HasExited)
                {
                    process.Kill(true);
                }
            }
            catch
            {
                // Ignore
            }
            return false;
        }
    }

    public static async Task<bool> ConnectSCUNETWifi()
    {
        if (!Utils.IsWindows()) return false;
        using var process = new Process();
        process.StartInfo = new ProcessStartInfo
        {
            FileName = "netsh",
            ArgumentList = { "wlan", "connect", "name=SCUNET" },
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            UseShellExecute = false,
            CreateNoWindow = true
        };
        process.Start();
        var stdOutTask = process.StandardOutput.ReadToEndAsync();
        var stdErrTask = process.StandardError.ReadToEndAsync();
        using var cts = new CancellationTokenSource(8000);
        await process.WaitForExitAsync(cts.Token);
        try
        {
            await process.WaitForExitAsync(cts.Token);
            var stdout = await stdOutTask;
            var stderr = await stdErrTask;
            var output = stderr + stdout;
            return output.Contains("已成功连接", StringComparison.OrdinalIgnoreCase)
                   || output.Contains("已连接", StringComparison.OrdinalIgnoreCase)
                   || output.Contains("success", StringComparison.OrdinalIgnoreCase);
        }
        catch (OperationCanceledException)
        {
            try
            {
                if (!process.HasExited)
                {
                    process.Kill(true);
                }
            }
            catch
            {
                // Ignore
            }
            return false;
        }
    }
}