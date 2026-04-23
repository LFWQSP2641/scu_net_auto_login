using Avalonia.Media;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using System.Collections.ObjectModel;
using Avalonia;

namespace Sal.GUI.ViewModels;

public partial class MainWindowViewModel : ViewModelBase
{
    [ObservableProperty]
    public partial bool IsPaneOpen { get; set; } = true;

    public ObservableCollection<NavItem> NavItems { get; } = [];

    [ObservableProperty]
    public partial NavItem? SelectedNavItem { get; set; }

    [ObservableProperty]
    public partial ViewModelBase? CurrentPage { get; set; }

    public MainWindowViewModel()
    {
        NavItems.Add(new NavItem
        {
            Title = "Config",
            IconGeometry = ResolveIconGeometry("SemiIconSettingStroked")
        });

        NavItems.Add(new NavItem
        {
            Title = "Login",
            IconGeometry = ResolveIconGeometry("SemiIconUserStroked")
        });

        SelectedNavItem = NavItems[0];
    }

    private static Geometry? ResolveIconGeometry(string resourceKey)
    {
        if (Application.Current is null)
        {
            return null;
        }

        return Application.Current.TryGetResource(resourceKey, theme: null, out var resource)
            ? resource as Geometry
            : null;
    }

    [RelayCommand]
    private void TogglePane() => IsPaneOpen = !IsPaneOpen;

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
        CurrentPage = value.Title switch
        {
            "Config" => new ConfigEditViewModel(),
            "Login" => new LoginViewModel(),
            _ => null
        };
    }
}

public class NavItem
{
    public string Title { get; set; } = string.Empty;
    public Geometry? IconGeometry { get; set; }
}
