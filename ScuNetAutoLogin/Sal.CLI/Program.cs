using ConsoleAppFramework;
using Sal.CLI;
using ServiceLib.Manager;

AppManager.Instance.InitApp();

var app = ConsoleApp.Create();
app.Add<MyCommands>();
app.Run(args);
