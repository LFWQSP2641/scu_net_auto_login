using System.Collections.Concurrent;
using System.Reflection;

namespace ServiceLib.Common;

public class EmbedUtils
{
    private static readonly ConcurrentDictionary<string, string> _dicEmbedCache = new();

    /// <summary>
    /// Get embedded text resources
    /// </summary>
    /// <param name="res"></param>
    /// <returns></returns>
    public static string GetEmbedText(string res)
    {
        if (_dicEmbedCache.TryGetValue(res, out var value))
        {
            return value;
        }

        var assembly = Assembly.GetExecutingAssembly();
        using var stream = assembly.GetManifestResourceStream(res);
        ArgumentNullException.ThrowIfNull(stream);
        using StreamReader reader = new(stream);
        var result = reader.ReadToEnd();

        _dicEmbedCache.TryAdd(res, result);
        return result;
    }
}