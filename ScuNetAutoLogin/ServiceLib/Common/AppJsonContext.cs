using ServiceLib.Data;
using System.Text.Json.Serialization;

namespace ServiceLib.Common;

[JsonSerializable(typeof(AccountItem))]
[JsonSerializable(typeof(List<AccountItem>))]
[JsonSerializable(typeof(ConfigItem))]
internal partial class AppJsonContext : JsonSerializerContext;