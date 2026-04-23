using CommunityToolkit.Mvvm.ComponentModel;
using ServiceLib.Data;
using System.Collections.ObjectModel;
using System.Linq;

namespace Sal.GUI.Models;

public partial class ConfigModel : ObservableObject
{
    public ConfigModel()
    { }

    public ConfigModel(ConfigItem configItem)
    {
        InitialDelayMs = configItem.InitialDelayMs;
        RetryCount = configItem.RetryCount;
        RetryDelayMs = configItem.RetryDelayMs;
        UserList = new ObservableCollection<AccountModel>(configItem.UserList.Select(x => new AccountModel
        {
            Remark = x.Remark,
            Username = x.Username,
            Password = x.Password,
            Service = x.Service
        }));
        EnableHotspot = configItem.EnableHotspot;
        ConnectSCUNETWifi = configItem.ConnectSCUNETWifi;
    }

    [ObservableProperty]
    public partial int InitialDelayMs { get; set; }

    [ObservableProperty]
    public partial int RetryCount { get; set; }

    [ObservableProperty]
    public partial int RetryDelayMs { get; set; }

    public ObservableCollection<AccountModel> UserList { get; set; } = [];

    [ObservableProperty]
    public partial bool EnableHotspot { get; set; }

    [ObservableProperty]
    public partial bool ConnectSCUNETWifi { get; set; }

    public ConfigItem ToConfigItem()
    {
        return new ConfigItem
        {
            InitialDelayMs = InitialDelayMs,
            RetryCount = RetryCount,
            RetryDelayMs = RetryDelayMs,
            UserList = UserList.Select(x => new AccountItem
            {
                Remark = x.Remark,
                Username = x.Username,
                Password = x.Password,
                Service = x.Service
            }).ToList(),
            EnableHotspot = EnableHotspot,
            ConnectSCUNETWifi = ConnectSCUNETWifi
        };
    }
}