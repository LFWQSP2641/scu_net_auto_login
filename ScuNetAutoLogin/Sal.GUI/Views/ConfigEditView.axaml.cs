using Avalonia;
using Avalonia.Controls;
using Sal.GUI.ViewModels;

namespace Sal.GUI.Views;

public partial class ConfigEditView : UserControl
{
    public ConfigEditView()
    {
        InitializeComponent();
        DetachedFromVisualTree += OnDetachedFromVisualTree;
    }

    private async void OnDetachedFromVisualTree(object? sender, VisualTreeAttachmentEventArgs e)
    {
        try
        {
            DetachedFromVisualTree -= OnDetachedFromVisualTree;

            if (DataContext is ConfigEditViewModel vm)
            {
                await vm.OnViewClosingAsync();
            }
        }
        catch
        {
            // Ignore
        }
    }
}