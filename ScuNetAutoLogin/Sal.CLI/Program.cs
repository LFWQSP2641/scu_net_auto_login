using ConsoleAppFramework;
using Sal.CLI;
using ServiceLib.Manager;

AppManager.Instance.InitApp();
await AppManager.Instance.SetConfig(AppManager.Instance.GetConfig());

var app = ConsoleApp.Create();
app.Add<MyCommands>();
app.Run(args);
