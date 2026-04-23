using Avalonia.Controls;
using Avalonia.Controls.Templates;
using CommunityToolkit.Mvvm.ComponentModel;
using Sal.GUI.ViewModels;
using Sal.GUI.Views;
using System;
using System.Collections.Generic;

namespace Sal.GUI;

/// <summary>
/// Given a view model, returns the corresponding view if possible.
/// </summary>
public class ViewLocator : IDataTemplate
{
    private readonly Dictionary<Type, Func<Control?>> _locator = new();

    public ViewLocator()
    {
        RegisterViewFactory<MainWindowViewModel, MainWindowView>();
        RegisterViewFactory<ConfigEditViewModel, ConfigEditView>();
        RegisterViewFactory<LoginViewModel, LoginView>();
    }

    public Control Build(object? data)
    {
        if (data is null)
            return new TextBlock { Text = $"No VM provided" };

        _locator.TryGetValue(data.GetType(), out var factory);

#if DEBUG
        if (factory == null)
        {
            throw new InvalidOperationException($"No view registered for VM type: {data.GetType()}");
        }
#endif
        return factory?.Invoke() ?? new TextBlock { Text = $"VM Not Registered: {data.GetType()}" };
    }

    public bool Match(object? data)
    {
        return data is ObservableObject;
    }

    public void RegisterViewFactory<TViewModel>(Func<Control> factory) where TViewModel : class => _locator.Add(typeof(TViewModel), factory);

    public void RegisterViewFactory<TViewModel, TView>()
        where TViewModel : class
        where TView : Control, new()
        => _locator.Add(typeof(TViewModel), () => new TView());
}

