using ServiceLib.Data;

namespace ServiceLib.Manager;

public class AppManager
{
    private static readonly Lazy<AppManager> _instance = new(() => new AppManager());
    public static AppManager Instance => _instance.Value;

    private ConfigItem _config = new();
    public ConfigItem GetConfig() => _config;
    public async Task SetConfig(ConfigItem config)
    {
        _config = config;
        await ConfigManager.Instance.SaveConfig(config);
    }

    public bool InitApp()
    {
        try
        {
            _config = ConfigManager.Instance.LoadConfig();
            return true;
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Failed to load config: {ex.Message}");
            return false;
        }
    }
}