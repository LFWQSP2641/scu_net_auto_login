using System.Text.Json.Serialization;
using ServiceLib.Data;

namespace ServiceLib.Common;

[JsonSerializable(typeof(AccountItem))]
[JsonSerializable(typeof(List<AccountItem>))]
[JsonSerializable(typeof(ConfigItem))]
internal partial class AppJsonContext : JsonSerializerContext;
