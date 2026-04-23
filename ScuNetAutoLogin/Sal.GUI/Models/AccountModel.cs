using CommunityToolkit.Mvvm.ComponentModel;
using ServiceLib.Data;

namespace Sal.GUI.Models;

public partial class AccountModel : ObservableObject
{
    public AccountModel()
    { }

    public AccountModel(AccountItem accountItem)
    {
        Remark = accountItem.Remark;
        Username = accountItem.Username;
        Password = accountItem.Password;
        Service = accountItem.Service;
    }

    [ObservableProperty]
    public partial string Remark { get; set; } = string.Empty;

    [ObservableProperty]
    public partial string Username { get; set; } = string.Empty;

    [ObservableProperty]
    public partial string Password { get; set; } = string.Empty;

    [ObservableProperty]
    public partial string Service { get; set; } = string.Empty;

    public AccountItem ToAccountItem()
    {
        return new AccountItem
        {
            Remark = Remark,
            Username = Username,
            Password = Password,
            Service = Service
        };
    }
}