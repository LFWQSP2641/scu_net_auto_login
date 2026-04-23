using System.Text.Encodings.Web;
using System.Text.Json;
using System.Text.Json.Nodes;
using System.Text.Json.Serialization;
using System.Text.Json.Serialization.Metadata;
using ServiceLib.Data;

namespace ServiceLib.Common;

public class JsonUtils
{
    private static readonly JsonSerializerOptions _defaultDeserializeOptions = new()
    {
        PropertyNameCaseInsensitive = true,
        ReadCommentHandling = JsonCommentHandling.Skip
    };

    private static readonly JsonSerializerOptions _defaultSerializeOptions = new()
    {
        WriteIndented = true,
        DefaultIgnoreCondition = JsonIgnoreCondition.WhenWritingNull,
        Encoder = JavaScriptEncoder.UnsafeRelaxedJsonEscaping
    };

    private static readonly JsonSerializerOptions _nullValueSerializeOptions = new()
    {
        WriteIndented = true,
        DefaultIgnoreCondition = JsonIgnoreCondition.Never,
        Encoder = JavaScriptEncoder.UnsafeRelaxedJsonEscaping
    };

    private static readonly JsonDocumentOptions _defaultDocumentOptions = new()
    {
        CommentHandling = JsonCommentHandling.Skip
    };

    /// <summary>
    /// DeepCopy
    /// </summary>
    /// <typeparam name="T"></typeparam>
    /// <param name="obj"></param>
    /// <returns></returns>
    public static T? DeepCopy<T>(T? obj)
    {
        if (obj is null)
        {
            return default;
        }

        var typeInfo = AppJsonContext.Default.GetTypeInfo(typeof(T)) as JsonTypeInfo<T>;
        if (typeInfo is null)
        {
            return default;
        }

        return Deserialize(Serialize(obj, typeInfo, indented: false), typeInfo);
    }

    /// <summary>
    /// Deserialize to object
    /// </summary>
    /// <typeparam name="T"></typeparam>
    /// <param name="strJson"></param>
    /// <returns></returns>
    public static T? Deserialize<T>(string? strJson)
    {
        var typeInfo = AppJsonContext.Default.GetTypeInfo(typeof(T)) as JsonTypeInfo<T>;
        if (typeInfo is null)
        {
            return default;
        }

        return Deserialize(strJson, typeInfo);
    }

    public static T? Deserialize<T>(string? strJson, JsonTypeInfo<T> typeInfo)
    {
        try
        {
            if (string.IsNullOrWhiteSpace(strJson))
            {
                return default;
            }

            return JsonSerializer.Deserialize(strJson, typeInfo);
        }
        catch
        {
            return default;
        }
    }

    /// <summary>
    /// parse
    /// </summary>
    /// <param name="strJson"></param>
    /// <returns></returns>
    public static JsonNode? ParseJson(string? strJson)
    {
        try
        {
            if (string.IsNullOrWhiteSpace(strJson))
            {
                return null;
            }
            return JsonNode.Parse(strJson, nodeOptions: null, _defaultDocumentOptions);
        }
        catch
        {
            //SaveLog(ex.Message, ex);
            return null;
        }
    }

    /// <summary>
    /// Serialize Object to Json string
    /// </summary>
    /// <param name="obj"></param>
    /// <param name="indented"></param>
    /// <param name="nullValue"></param>
    /// <returns></returns>
    public static string Serialize(object? obj, bool indented = true, bool nullValue = false)
    {
        if (obj == null)
        {
            return string.Empty;
        }

        var typeInfo = AppJsonContext.Default.GetTypeInfo(obj.GetType());
        if (typeInfo is null)
        {
            return string.Empty;
        }

        var result = string.Empty;
        try
        {
            var options = BuildSerializeOptions(indented, nullValue);
            result = JsonSerializer.Serialize(obj, typeInfo);
            if (!options.WriteIndented || options.DefaultIgnoreCondition != JsonIgnoreCondition.WhenWritingNull)
            {
                var node = JsonNode.Parse(result, documentOptions: _defaultDocumentOptions);
                result = node?.ToJsonString(options) ?? result;
            }
        }
        catch
        {
            // Ignore
        }
        return result;
    }

    /// <summary>
    /// Serialize Object to Json string
    /// </summary>
    /// <param name="obj"></param>
    /// <param name="options"></param>
    /// <returns></returns>
    public static string Serialize(object? obj, JsonSerializerOptions? options)
    {
        if (obj == null)
        {
            return string.Empty;
        }

        var typeInfo = AppJsonContext.Default.GetTypeInfo(obj.GetType());
        if (typeInfo is null)
        {
            return string.Empty;
        }

        var result = string.Empty;
        try
        {
            result = JsonSerializer.Serialize(obj, typeInfo);

            var effectiveOptions = options ?? _defaultSerializeOptions;
            var node = JsonNode.Parse(result, documentOptions: _defaultDocumentOptions);
            result = node?.ToJsonString(effectiveOptions) ?? result;
        }
        catch
        {
            // Ignore
        }
        return result;
    }

    public static string Serialize<T>(T? obj, JsonTypeInfo<T> typeInfo, bool indented = true, bool nullValue = false)
    {
        try
        {
            if (obj == null)
            {
                return string.Empty;
            }

            var options = BuildSerializeOptions(indented, nullValue);
            var json = JsonSerializer.Serialize(obj, typeInfo);

            if (options.WriteIndented && options.DefaultIgnoreCondition == JsonIgnoreCondition.WhenWritingNull)
            {
                return json;
            }

            var node = JsonNode.Parse(json, documentOptions: _defaultDocumentOptions);
            return node?.ToJsonString(options) ?? json;
        }
        catch
        {
            return string.Empty;
        }
    }

    /// <summary>
    /// SerializeToNode
    /// </summary>
    /// <param name="obj"></param>
    /// <param name="options"></param>
    /// <returns></returns>
    public static JsonNode? SerializeToNode(object? obj, JsonSerializerOptions? options = null)
    {
        if (obj == null)
        {
            return null;
        }

        var typeInfo = AppJsonContext.Default.GetTypeInfo(obj.GetType());
        if (typeInfo is null)
        {
            return null;
        }

        try
        {
            var json = JsonSerializer.Serialize(obj, typeInfo);
            return JsonNode.Parse(json, nodeOptions: null, _defaultDocumentOptions);
        }
        catch
        {
            return null;
        }
    }

    private static JsonSerializerOptions BuildSerializeOptions(bool indented, bool nullValue)
    {
        return new JsonSerializerOptions(_defaultSerializeOptions)
        {
            WriteIndented = indented,
            DefaultIgnoreCondition = nullValue ? JsonIgnoreCondition.Never : JsonIgnoreCondition.WhenWritingNull
        };
    }
}