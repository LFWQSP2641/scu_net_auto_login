using System.Text.Json;
using ServiceLib.Common;
using ServiceLib.Data;

namespace ServiceLib.Manager;

public class ConfigManager
{
    private static readonly Lazy<ConfigManager> _instance = new(() => new ConfigManager());
    public static ConfigManager Instance => _instance.Value;

    private readonly SemaphoreSlim _fileLock = new(1, 1);

    public ConfigItem LoadConfig()
    {
        ConfigItem? config = null;
        try
        {
            var filePath = Path.Combine(Utils.GetConfigPath(), "config.json");
            var fileContent = File.ReadAllText(filePath);
            config = JsonSerializer.Deserialize(fileContent, AppJsonContext.Default.ConfigItem);
        }
        catch
        {
            // Ignore
        }
        config ??= new();
        return config;
    }

    public async Task SaveConfig(ConfigItem config)
    {
        await _fileLock.WaitAsync();
        try
        {
            var filePath = Path.Combine(Utils.GetConfigPath(), "config.json");
            var tempFilePath = filePath + ".tmp";
            var jsonContent = JsonSerializer.Serialize(config, AppJsonContext.Default.ConfigItem);
            await File.WriteAllTextAsync(tempFilePath, jsonContent);
            File.Move(tempFilePath, filePath, true);
        }
        catch
        {
            // Ignore
        }
        finally
        {
            _fileLock.Release();
        }
    }
}