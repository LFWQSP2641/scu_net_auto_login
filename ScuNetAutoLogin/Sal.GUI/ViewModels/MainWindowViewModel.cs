using System.Collections.ObjectModel;
using Avalonia;
using Avalonia.Media;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using Sal.GUI.Resx;

namespace Sal.GUI.ViewModels;

public partial class MainWindowViewModel : ViewModelBase
{
    public MainWindowViewModel()
    {
        NavItems.Add(new NavItem
        {
            Id = "config",
            Title = ResUI.NavConfig,
            IconGeometry = ResolveIconGeometry("SemiIconSettingStroked"),
        });

        NavItems.Add(new NavItem
        {
            Id = "login",
            Title = ResUI.NavLogin,
            IconGeometry = ResolveIconGeometry("SemiIconUserStroked"),
        });

        SelectedNavItem = NavItems[0];
    }

    [ObservableProperty] public partial bool IsPaneOpen { get; set; } = true;

    public ObservableCollection<NavItem> NavItems { get; } = [];

    [ObservableProperty] public partial NavItem? SelectedNavItem { get; set; }

    [ObservableProperty] public partial ViewModelBase? CurrentPage { get; set; }

    private static Geometry? ResolveIconGeometry(string resourceKey)
    {
        if (Application.Current is null)
        {
            return null;
        }

        return Application.Current.TryGetResource(resourceKey, null, out var resource)
            ? resource as Geometry
            : null;
    }

    [RelayCommand]
    private void TogglePane()
    {
        IsPaneOpen = !IsPaneOpen;
    }

    public void UpdateLayout(double width)
    {
        IsPaneOpen = width > 600;
    }

    partial void OnSelectedNavItemChanged(NavItem? value)
    {
        if (value is null)
        {
            return;
        }
        CurrentPage = value.Id switch
        {
            "config" => new ConfigEditViewModel(),
            "login" => new LoginViewModel(),
            _ => null,
        };
    }
}

public class NavItem
{
    public string Id { get; set; } = string.Empty;
    public string Title { get; set; } = string.Empty;
    public Geometry? IconGeometry { get; set; }
}
