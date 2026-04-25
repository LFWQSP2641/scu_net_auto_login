using System.Collections.Generic;

namespace Sal.GUI.Models;

public sealed record ServiceOption(string Id, string Name)
{
    public static List<ServiceOption> CreateDefaultList () =>
    [
        new("edunet", Resx.ResUI.LbEdunet),
        new("chinatelecom", Resx.ResUI.LbChinaTelecom),
        new("chinamobile", Resx.ResUI.LbChinaMobile),
        new("chinaunicom", Resx.ResUI.LbChinaUnicom),
    ];
}