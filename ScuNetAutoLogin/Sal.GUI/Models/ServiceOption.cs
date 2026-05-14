using System.Collections.Generic;
using Sal.GUI.Resx;

namespace Sal.GUI.Models;

public sealed record ServiceOption(string Id, string Name)
{
    public static List<ServiceOption> CreateDefaultList()
    {
        return
        [
            new("edunet", ResUI.LbEdunet),
            new("chinatelecom", ResUI.LbChinaTelecom),
            new("chinamobile", ResUI.LbChinaMobile),
            new("chinaunicom", ResUI.LbChinaUnicom),
        ];
    }
}
