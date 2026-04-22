using ConsoleAppFramework;
using ServiceLib.Data;
using ServiceLib.Resx;
using ServiceLib.Service;

namespace Sal.CLI;

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
    [Command("login|l")]
    public async Task Login(
        string username,
        string password,
        string service,
        int retryCount = 1,
        int retryDelay = 5,
        int initialDelay = 0)
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
}