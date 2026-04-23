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
            config = JsonUtils.Deserialize<ConfigItem>(fileContent);
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
            var filePath = Path.Combine(Utils.GetConfigPath(), Global.ConfigFileName);
            var directoryPath = Path.GetDirectoryName(filePath);
            if (!Directory.Exists(directoryPath))
            {
                Directory.CreateDirectory(directoryPath!);
            }
            var tempFilePath = filePath + ".tmp";
            var jsonContent = JsonUtils.Serialize(config);
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