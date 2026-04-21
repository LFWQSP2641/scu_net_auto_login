namespace ServiceLib.Data;

public record AccountItem
{
    public string Remark { get; init; } = string.Empty;
    public string Username { get; init; } = string.Empty;
    public string Password { get; init; } = string.Empty;
    public string Service { get; init; } = string.Empty;
}