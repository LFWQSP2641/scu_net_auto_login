using System;
using System.Threading.Tasks;
using ConsoleAppFramework;
using ServiceLib.Data;
using ServiceLib.Helper;
using ServiceLib.Resx;
using ServiceLib.Service;

namespace Sal.GUI.CLICommand;

public class MyCommands
{
    /// <summary>Login using config command.</summary>
    [Command("")]
    public async Task AutoLogin()
    {
        var autoLoginService = new AutoLoginService();
        var result = await autoLoginService.StartAutoLogin();
        if (result.Success)
        {
            Console.WriteLine(ResStr.AutoLoginSuccess);
            Console.WriteLine(ResStr.Details);
            foreach (var message in result.Message)
            {
                Console.WriteLine(message);
            }
        }
        else
        {
            Console.WriteLine(ResStr.AutoLoginFailed);
            Console.WriteLine(ResStr.Details);
            foreach (var error in result.Errors)
            {
                Console.WriteLine(error);
            }
        }
    }

    /// <summary>Login command.</summary>
    /// <param name="username">-u, Username for login.</param>
    /// <param name="password">-p, Password for login.</param>
    /// <param name="service">-s, Service type for login.</param>
    /// <param name="retryCount">-r, Number of retry attempts if login fails.</param>
    /// <param name="retryDelay">-d, Delay in seconds between retry attempts. (seconds)</param>
    /// <param name="initialDelay">-i, Initial delay in seconds before starting the login process. (seconds)</param>
    /// <param name="enableHotspot">-h, Enable hotspot if login succeeds.</param>
    /// <param name="connectSCUNETWifi">-c, Connect to SCUNET Wi-Fi before login.</param>
    [Command("login|l")]
    public async Task Login(
        string username,
        string password,
        string service,
        int retryCount = 1,
        int retryDelay = 5,
        int initialDelay = 0,
        bool enableHotspot = false,
        bool connectSCUNETWifi = false)
    {
        var account = new AccountItem()
        {
            Username = username,
            Password = password,
            Service = service,
        };
        var config = new ConfigItem()
        {
            UserList = [account],
            RetryCount = retryCount,
            RetryDelayMs = retryDelay * 1000,
            InitialDelayMs = initialDelay * 1000,
            EnableHotspot = enableHotspot,
            ConnectSCUNETWifi = connectSCUNETWifi,
        };
        var autoLoginService = new AutoLoginService();
        var result = await autoLoginService.StartAutoLogin(config);
        if (result.Success)
        {
            Console.WriteLine(ResStr.AutoLoginSuccess);
            Console.WriteLine(ResStr.Details);
            foreach (var message in result.Message)
            {
                Console.WriteLine(message);
            }
        }
        else
        {
            Console.WriteLine(ResStr.AutoLoginFailed);
            Console.WriteLine(ResStr.Details);
            foreach (var error in result.Errors)
            {
                Console.WriteLine(error);
            }
        }
    }

    /// <summary>
    /// Start a Wi-Fi hotspot on the device.
    /// </summary>
    [Command("hotspot|h")]
    public async Task Hotspot()
    {
        var success = await PlatformHelper.OpenHotspots();
        if (success)
        {
            Console.WriteLine(ResStr.HotspotStarted);
        }
        else
        {
            Console.WriteLine(ResStr.HotspotStartFailed);
        }
    }

    /// <summary>
    /// Connect to SCUNET Wi-Fi network.
    /// </summary>
    [Command("connect-scunet|c")]
    public async Task ConnectSCUNET()
    {
        var success = await PlatformHelper.ConnectSCUNETWifi();
        if (success)
        {
            Console.WriteLine(ResStr.SCUNETWifiConnected);
        }
        else
        {
            Console.WriteLine(ResStr.SCUNETWifiConnectFailed);
        }
    }
}