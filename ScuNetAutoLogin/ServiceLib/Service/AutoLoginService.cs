using ServiceLib.Data;
using ServiceLib.Manager;
using ServiceLib.Resx;

namespace ServiceLib.Service;

public record AutoLoginServiceResult
{
    public bool Success => Errors.Count == 0;

    public List<string> Errors { get; init; } = [];

    public List<string> Message { get; init; } = [];
}

public class AutoLoginService
{
    public async Task<AutoLoginServiceResult> StartAutoLogin()
    {
        var config = AppManager.Instance.GetConfig();
        return await StartAutoLogin(config);
    }

    public async Task<AutoLoginServiceResult> StartAutoLogin(ConfigItem config)
    {
        var result = new AutoLoginServiceResult();
        if (config.UserList.Count == 0)
        {
            result.Errors.Add(ResStr.NoUserConfigured);
            return result;
        }
        var message = new List<string>();
        var loginSuccess = false;
        var retryCount = config.RetryCount;
        retryCount = retryCount > 0 ? retryCount : 1;
        foreach (var user in config.UserList)
        {
            foreach (var _ in Enumerable.Range(1, retryCount))
            {
                try
                {
                    var loginService = new LoginService();
                    await loginService.StartLogin(user);
                }
                catch (LoginException.AlreadyLoggedIn ex)
                {
                    // already logged in, consider it a success and break the loop
                    message.Add(ex.Message);
                    loginSuccess = true;
                    break;
                }
                catch (Exception ex)
                {
                    // log the error and continue with the next user
                    message.Add(string.Format(ResStr.LoginFailedForUserFormat, user.Username, ex.Message));
                    if (config.RetryDelayMs > 0)
                    {
                        await Task.Delay(config.RetryDelayMs);
                    }
                }
                // no error, break the loop and return success
                loginSuccess = true;
                break;
            }
        }
        if (loginSuccess)
        {
            message.Add(ResStr.AutoLoginSuccess);
            result = result with { Message = message };
        }
        else
        {
            message.Add(ResStr.AutoLoginFailed);
            result = result with { Errors = message };
        }
        return result;
    }
}