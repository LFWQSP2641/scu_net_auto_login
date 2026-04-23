using Avalonia;
using ConsoleAppFramework;
using Sal.GUI.CLICommand;
using ServiceLib.Manager;
using System;
using System.Linq;

namespace Sal.GUI;

internal sealed class Program
{
    // Initialization code. Don't use any Avalonia, third-party APIs or any
    // SynchronizationContext-reliant code before AppMain is called: things aren't initialized
    // yet and stuff might break.
    [STAThread]
    public static void Main(string[] args)
    {
        AppManager.Instance.InitApp();
        if (args.FirstOrDefault() == "cli")
        {
            // Run CLI command
            var app = ConsoleApp.Create();
            app.Add<MyCommands>();
            app.Run(args.Skip(1).ToArray());
            return;
        }

        // Run GUI
        BuildAvaloniaApp().StartWithClassicDesktopLifetime(args);
    }

    // Avalonia configuration, don't remove; also used by visual designer.
    public static AppBuilder BuildAvaloniaApp()
        => AppBuilder.Configure<App>()
            .UsePlatformDetect()
#if DEBUG
            .WithDeveloperTools()
#endif
            .LogToTrace();
}