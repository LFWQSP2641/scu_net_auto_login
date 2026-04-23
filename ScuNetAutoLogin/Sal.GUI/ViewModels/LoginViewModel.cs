using System;
using CommunityToolkit.Mvvm.ComponentModel;
using Sal.GUI.Models;
using ServiceLib.Manager;
using System.Linq;
using ServiceLib.Data;
using System.Threading.Tasks;
using CommunityToolkit.Mvvm.Input;
using ServiceLib.Service;

namespace Sal.GUI.ViewModels;

public partial class LoginViewModel : ViewModelBase
{
    [ObservableProperty]
    [NotifyCanExecuteChangedFor(nameof(LoginCommand))]
    public partial AccountModel Account { get; set; }

    [ObservableProperty]
    [NotifyPropertyChangedFor(nameof(HasLoginMessage))]
    public partial string LoginMessage { get; set; } = string.Empty;

    public bool HasLoginMessage => !string.IsNullOrEmpty(LoginMessage);

    public LoginViewModel()
    {
        var configItem = AppManager.Instance.GetConfig();
        Account = new(configItem.UserList.FirstOrDefault(new AccountItem()));
    }

    public bool CanLogin() =>
        !string.IsNullOrEmpty(Account.Username)
        && !string.IsNullOrEmpty(Account.Password)
        && !string.IsNullOrEmpty(Account.Service);

    [RelayCommand(CanExecute = nameof(CanLogin))]
    public async Task LoginAsync()
    {
        LoginMessage = string.Empty;
        var accountItem = Account.ToAccountItem();
        var loginService = new LoginService();
        try
        {
            await loginService.StartLogin(accountItem);
            LoginMessage = "Login successful!";
        }
        catch (Exception ex)
        {
            LoginMessage = ex.Message;
        }
    }
}