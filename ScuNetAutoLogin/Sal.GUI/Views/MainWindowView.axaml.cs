using Avalonia.Controls;
using Sal.GUI.ViewModels;

namespace Sal.GUI.Views;

public partial class MainWindowView : Window
{
    public MainWindowView()
    {
        InitializeComponent();
        SizeChanged += (_, e) =>
        {
            if (DataContext is MainWindowViewModel vm)
            {
                vm.UpdateLayout(e.NewSize.Width);
            }
        };
    }
}