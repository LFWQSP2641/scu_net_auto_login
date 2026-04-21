namespace ServiceLib.Data;

public record ConfigItem
{
    public int InitialDelayMs { get; init; }
    public int RetryCount { get; init; }
    public int RetryDelayMs { get; init; }
    public List<AccountItem> UserList { get; init; } = [];
}