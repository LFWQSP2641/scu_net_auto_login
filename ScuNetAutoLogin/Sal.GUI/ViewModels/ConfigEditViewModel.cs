using CommunityToolkit.Mvvm.ComponentModel;
using Sal.GUI.Models;
using Sal.GUI.Resx;
using ServiceLib.Common;
using ServiceLib.Data;
using ServiceLib.Manager;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;

namespace Sal.GUI.ViewModels;

public sealed record ServiceOption(string Id, string Name);

public partial class ConfigEditViewModel : ViewModelBase
{
    private static readonly TimeSpan SaveCheckInterval = TimeSpan.FromMilliseconds(800);

    public static IReadOnlyList<ServiceOption> ServiceOptions { get; } =
    [
        new("edunet", ResUI.LbEdunet),
        new("chinatelecom", ResUI.LbChinaTelecom),
        new("chinamobile", ResUI.LbChinaMobile),
        new("chinaunicom", ResUI.LbChinaUnicom),
    ];

    private readonly SemaphoreSlim _saveGate = new(1, 1);
    private readonly CancellationTokenSource _monitorCts = new();
    private readonly Task _monitorTask;
    private string _lastSavedSnapshot;
    private int _isClosing;

    [ObservableProperty]
    public partial ConfigModel Config { get; set; }

    [ObservableProperty]
    public partial AccountModel Account { get; set; }

    public ConfigEditViewModel()
    {
        Config = new(AppManager.Instance.GetConfig());
        if (Config.UserList.Count == 0)
        {
            Config.UserList.Add(new AccountModel());
        }
        Account = Config.UserList.First();

        _lastSavedSnapshot = CreateSnapshot(Config.ToConfigItem());
        _monitorTask = MonitorConfigChangesAsync(_monitorCts.Token);
    }

    public async Task OnViewClosingAsync()
    {
        if (Interlocked.Exchange(ref _isClosing, 1) == 1)
        {
            return;
        }

        await _monitorCts.CancelAsync();
        try
        {
            await _monitorTask;
        }
        catch (OperationCanceledException)
        {
            // Ignore
        }

        await SaveIfChangedAsync(force: true);

        _monitorCts.Dispose();
        _saveGate.Dispose();
    }

    private async Task MonitorConfigChangesAsync(CancellationToken token)
    {
        using var timer = new PeriodicTimer(SaveCheckInterval);
        while (await timer.WaitForNextTickAsync(token))
        {
            await SaveIfChangedAsync();
        }
    }

    private async Task SaveIfChangedAsync(bool force = false)
    {
        if (!force && Volatile.Read(ref _isClosing) == 1)
        {
            return;
        }

        var configItem = Config.ToConfigItem();
        var snapshot = CreateSnapshot(configItem);
        if (snapshot == _lastSavedSnapshot)
        {
            return;
        }

        await _saveGate.WaitAsync();
        try
        {
            configItem = Config.ToConfigItem();
            snapshot = CreateSnapshot(configItem);
            if (snapshot == _lastSavedSnapshot)
            {
                return;
            }

            await AppManager.Instance.SetConfig(configItem);
            _lastSavedSnapshot = snapshot;
        }
        finally
        {
            _saveGate.Release();
        }
    }

    private static string CreateSnapshot(ConfigItem configItem)
    {
        return JsonUtils.Serialize(configItem, indented: false);
    }
}