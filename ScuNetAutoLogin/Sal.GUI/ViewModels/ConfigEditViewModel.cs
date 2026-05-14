using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using Sal.GUI.Models;
using ServiceLib.Common;
using ServiceLib.Data;
using ServiceLib.Helper;
using ServiceLib.Manager;

namespace Sal.GUI.ViewModels;

public partial class ConfigEditViewModel : ViewModelBase
{
    private static readonly TimeSpan SaveCheckInterval = TimeSpan.FromMilliseconds(800);
    private readonly CancellationTokenSource _monitorCts = new();
    private readonly Task _monitorTask;

    private readonly SemaphoreSlim _saveGate = new(1, 1);
    private int _isClosing;
    private string _lastSavedSnapshot;

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

    public static IReadOnlyList<ServiceOption> ServiceOptions { get; } = ServiceOption.CreateDefaultList();

    [ObservableProperty] public partial ConfigModel Config { get; set; }

    [ObservableProperty] public partial AccountModel Account { get; set; }

    public static bool CanManageStartup()
    {
        return Utils.IsWindows();
    }

    [RelayCommand(CanExecute = nameof(CanManageStartup))]
    public void AddWindowsStartup()
    {
        StartupHelper.AddStartup();
    }

    [RelayCommand(CanExecute = nameof(CanManageStartup))]
    public void RemoveWindowsStartup()
    {
        StartupHelper.RemoveStartup();
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

        await SaveIfChangedAsync(true);

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
        return JsonUtils.Serialize(configItem, false);
    }
}
