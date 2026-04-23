using System.Text.RegularExpressions;

namespace ServiceLib.Common;

public partial class LoginConstants
{
    public const string MainUrl = "http://192.168.2.135/";
    public const string LoginPath = "eportal/InterFace.do?method=login";
    public const string RedirectPortalHost = "123.123.123.123";

    public const string HttpHeaderUserAgent =
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/134.0.0.0 Safari/537.36 Edg/134.0.0.0";

    public const string HttpHeaderContentType =
        "application/x-www-form-urlencoded; charset=UTF-8";

    public const string HttpHeaderAccept = "*/*";

    public static readonly List<string> ServiceCodeList =
    [
        "chinatelecom",
        "chinamobile",
        "chinaunicom",
        "edunet",
    ];

    public static readonly Dictionary<string, string> ServiceCodeMap = new()
    {
        {"chinatelecom", "%E7%94%B5%E4%BF%A1%E5%87%BA%E5%8F%A3"},
        {"chinamobile", "%E7%A7%BB%E5%8A%A8%E5%87%BA%E5%8F%A3"},
        {"chinaunicom", "%E8%81%94%E9%80%9A%E5%87%BA%E5%8F%A3"},
        {"edunet", "internet"},
    };

    public static readonly List<Regex> QueryStringExtractPatterns =
    [
        QueryStringExtractPatterns1(),
        QueryStringExtractPatterns2()
    ];

    [GeneratedRegex(@"/index\.jsp\?([^'""#\s>]+)", RegexOptions.IgnoreCase)]
    private static partial Regex QueryStringExtractPatterns1();

    [GeneratedRegex(@"top\.self\.location\.href\s*=\s*['""][^'"" ]*/index\.jsp\?([^'"" ]+ )['""]", RegexOptions.IgnoreCase)]
    private static partial Regex QueryStringExtractPatterns2();
}