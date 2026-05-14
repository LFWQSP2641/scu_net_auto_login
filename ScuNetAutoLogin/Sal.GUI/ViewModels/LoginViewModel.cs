using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using Sal.GUI.Models;
using Sal.GUI.Resx;
using ServiceLib.Common;
using ServiceLib.Data;
using ServiceLib.Helper;
using ServiceLib.Manager;
using ServiceLib.Service;

namespace Sal.GUI.ViewModels;

public partial class LoginViewModel : ViewModelBase
{
    public LoginViewModel()
    {
        var configItem = AppManager.Instance.GetConfig();
        var account = configItem.UserList.FirstOrDefault(new AccountItem());
        Username = account.Username;
        Password = account.Password;
        Service = account.Service;
    }

    [ObservableProperty]
    [NotifyCanExecuteChangedFor(nameof(LoginCommand))]
    public partial string Username { get; set; } = string.Empty;

    [ObservableProperty]
    [NotifyCanExecuteChangedFor(nameof(LoginCommand))]
    public partial string Password { get; set; } = string.Empty;

    [ObservableProperty]
    [NotifyCanExecuteChangedFor(nameof(LoginCommand))]
    public partial string Service { get; set; } = string.Empty;

    [ObservableProperty]
    [NotifyPropertyChangedFor(nameof(HasLoginMessage))]
    public partial string LoginMessage { get; set; } = string.Empty;

    public static IReadOnlyList<ServiceOption> ServiceOptions { get; } = ServiceOption.CreateDefaultList();

    public bool HasLoginMessage => !string.IsNullOrEmpty(LoginMessage);

    public bool CanLogin()
    {
        return !string.IsNullOrEmpty(Username)
               && !string.IsNullOrEmpty(Password)
               && !string.IsNullOrEmpty(Service);
    }

    [RelayCommand(CanExecute = nameof(CanLogin))]
    public async Task LoginAsync()
    {
        LoginMessage = string.Empty;
        var accountItem = new AccountItem
        {
            Username = Username,
            Password = Password,
            Service = Service,
        };
        var loginService = new LoginService();
        try
        {
            await loginService.StartLogin(accountItem);
            LoginMessage = ResUI.MsgLoginSuccess;
        }
        catch (Exception ex)
        {
            LoginMessage = ex.Message;
        }
    }

    public static bool IsWindows()
    {
        return Utils.IsWindows();
    }

    public static bool CanOpenHotspot()
    {
        return IsWindows();
    }

    [RelayCommand(CanExecute = nameof(CanOpenHotspot))]
    public async Task OpenHotspotAsync()
    {
        await PlatformHelper.OpenHotspots();
    }

    public static bool CanConnectWifi()
    {
        return IsWindows();
    }

    [RelayCommand(CanExecute = nameof(CanConnectWifi))]
    public async Task ConnectSCUNETWifiAsync()
    {
        await PlatformHelper.ConnectSCUNETWifi();
    }
}
