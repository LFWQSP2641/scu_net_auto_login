using ServiceLib.Common;
using ServiceLib.Data;
using ServiceLib.Helper;
using ServiceLib.Resx;
using System.Net.Http.Headers;

namespace ServiceLib.Service;

public abstract class LoginException : Exception
{
    protected LoginException(string message, Exception? innerException = null)
        : base(message, innerException) { }

    public sealed class UnsupportedService(string service)
        : LoginException(string.Format(ResStr.ErrUnsupportedServiceFormat, service))
    {
        public string Service { get; } = service;
    }

    public sealed class MissingMacAddress()
        : LoginException(ResStr.ErrMissingMacAddress);

    public sealed class AlreadyLoggedIn()
        : LoginException(ResStr.ErrAlreadyLoggedIn);

    public sealed class RiskControlTriggered()
        : LoginException(ResStr.ErrRiskControlTriggered);

    public sealed class TooManyUsersOnline()
        : LoginException(ResStr.ErrTooManyUsersOnline);

    public sealed class MissingRedirectLocation(string step)
        : LoginException(string.Format(ResStr.ErrMissingRedirectLocationFormat, step))
    {
        public string Step { get; } = step;
    }

    public sealed class UnexpectedRedirect(string location)
        : LoginException(string.Format(ResStr.ErrUnexpectedRedirectFormat, location))
    {
        public string Location { get; } = location;
    }

    public sealed class QueryStringExtractionFailed(string reason)
        : LoginException(reason);

    public sealed class LoginFailed(string responseSnippet)
        : LoginException(string.Format(ResStr.ErrLoginFailedFormat, responseSnippet))
    {
        public string ResponseSnippet { get; } = responseSnippet;
    }
}

public class LoginService
{
    private readonly HttpClient _httpClient = new
    (
        new HttpClientHandler
        {
            AllowAutoRedirect = false
        }
    );

    public async Task StartLogin(AccountItem account)
    {
        var queryString = await GetLoginQueryString();
        await PostLoginRequest(queryString, account);
    }

    private async Task PostLoginRequest(string queryString, AccountItem account)
    {
        var loginPostUrl = new Uri(new Uri(LoginConstants.MainUrl), LoginConstants.LoginPath);
        var serviceCode = LoginConstants.ServiceCodeMap.GetValueOrDefault(account.Service) ?? throw new LoginException.UnsupportedService(account.Service);
        var macAddress = GetMacFromQueryString(queryString) ?? throw new LoginException.MissingMacAddress();

        var postData = new FormUrlEncodedContent(new Dictionary<string, string>
        {
            ["userId"] = account.Username,
            ["password"] = LegacyCampusRsaEncryptor.EncryptPassword(account.Password, macAddress),
            ["service"] = serviceCode,
            ["queryString"] = queryString,
            ["operatorPwd"] = string.Empty,
            ["operatorUserId"] = string.Empty,
            ["validcode"] = string.Empty,
            ["passwordEncrypt"] = "true",
        });
        var request = new HttpRequestMessage(HttpMethod.Post, loginPostUrl)
        {
            Content = postData
        };
        ApplyDefaultHeaders(request);

        var response = await _httpClient.SendAsync(request);
        var responseContent = await response.Content.ReadAsStringAsync();

        if (responseContent.Contains("\"result\":\"success\"", StringComparison.OrdinalIgnoreCase))
        {
        }
        else if (responseContent.Contains("\"message\":\"验证码错误.\"", StringComparison.OrdinalIgnoreCase))
        {
            throw new LoginException.RiskControlTriggered();
        }
        else if (responseContent.Contains("\"message\":\"你使用的账号已达到同时在线用户数量上限!\"", StringComparison.OrdinalIgnoreCase))
        {
            throw new LoginException.TooManyUsersOnline();
        }
        else
        {
            var snippet = responseContent.Length <= 200 ? responseContent : string.Concat(responseContent.AsSpan(0, 200), "...");
            throw new LoginException.LoginFailed(snippet);
        }
    }

    private async Task<string> GetLoginQueryString()
    {
        var request1 = new HttpRequestMessage(HttpMethod.Get, LoginConstants.MainUrl);
        ApplyDefaultHeaders(request1);
        var response1 = await _httpClient.SendAsync(request1);

        var redirect1Location = ResolveRedirectUrl(LoginConstants.MainUrl, response1.Headers.Location ?? throw new LoginException.MissingRedirectLocation("initial redirect"));

        if (!redirect1Location.Contains("eportal/redirectortosuccess.jsp", StringComparison.OrdinalIgnoreCase))
        {
            throw new LoginException.UnexpectedRedirect(redirect1Location);
        }

        var cookieHeader = MergeCookieHeader(string.Empty, response1.Headers);

        var request2 = new HttpRequestMessage(HttpMethod.Get, redirect1Location);
        ApplyDefaultHeaders(request2, cookieHeader);
        var response2 = await _httpClient.SendAsync(request2);

        var redirect2Location = ResolveRedirectUrl(redirect1Location, response2.Headers.Location ?? throw new LoginException.MissingRedirectLocation("second redirect"));

        if (redirect2Location.Contains("success.jsp", StringComparison.OrdinalIgnoreCase))
        {
            throw new LoginException.AlreadyLoggedIn();
        }
        else if (!redirect2Location.Contains(LoginConstants.RedirectPortalHost, StringComparison.OrdinalIgnoreCase))
        {
            throw new LoginException.UnexpectedRedirect(redirect2Location);
        }

        cookieHeader = MergeCookieHeader(cookieHeader, response2.Headers);

        var request3 = new HttpRequestMessage(HttpMethod.Get, redirect2Location);
        ApplyDefaultHeaders(request3, cookieHeader);
        var response3 = await _httpClient.SendAsync(request3);

        var htmlContent = await response3.Content.ReadAsStringAsync();
        return ExtractLoginQueryString(htmlContent);
    }

    private static string ExtractLoginQueryString(string htmlContent)
    {
        var normalizedHtml = htmlContent.Replace("&amp;", "&");

        foreach (var regex in LoginConstants.QueryStringExtractPatterns)
        {
            var match = regex.Match(normalizedHtml);
            if (match is not { Success: true, Groups.Count: > 1 }) continue;
            var value = match.Groups[1].Value.Trim();
            if (!string.IsNullOrEmpty(value))
                return value;
        }

        const string key = "/index.jsp?";
        var startIndex = normalizedHtml.IndexOf(key, StringComparison.OrdinalIgnoreCase);
        if (startIndex == -1)
        {
            throw new LoginException.QueryStringExtractionFailed("Failed to extract query string from login page");
        }

        var valueStart = startIndex + key.Length;

        var endIndices = new[]
        {
            normalizedHtml.IndexOf("'</script>", valueStart, StringComparison.OrdinalIgnoreCase),
            normalizedHtml.IndexOf('"', valueStart),
            normalizedHtml.IndexOf('\'', valueStart),
            normalizedHtml.IndexOf('<', valueStart),
            normalizedHtml.IndexOf(' ', valueStart)
        };

        var endIndex = endIndices
            .Where(i => i != -1)
            .Cast<int?>()
            .Min() ?? normalizedHtml.Length;

        var result = normalizedHtml.Substring(valueStart, endIndex - valueStart).Trim();

        if (string.IsNullOrEmpty(result))
        {
            throw new LoginException.QueryStringExtractionFailed("Extracted query string is empty");
        }

        return result;
    }

    private static void ApplyDefaultHeaders(HttpRequestMessage request, string? cookieHeader = null)
    {
        request.Headers.Add("User-Agent", LoginConstants.HttpHeaderUserAgent);
        request.Headers.Add("Accept", LoginConstants.HttpHeaderAccept);
        if (!string.IsNullOrWhiteSpace(cookieHeader))
        {
            request.Headers.Add("Cookie", cookieHeader);
        }
    }

    private static string ResolveRedirectUrl(string baseUrl, Uri locationHeader)
    {
        if (locationHeader.IsAbsoluteUri)
        {
            return locationHeader.ToString();
        }
        return new Uri(new Uri(baseUrl), locationHeader).ToString();
    }

    private static string MergeCookieHeader(string existingCookieHeader, HttpResponseHeaders headers)
    {
        var cookieMap = ParseCookieHeader(existingCookieHeader);

        if (!headers.TryGetValues("Set-Cookie", out var setCookieHeaders))
            return string.Join("; ", cookieMap.Select(kvp => $"{kvp.Key}={kvp.Value}"));
        foreach (var headerValue in setCookieHeaders)
        {
            var cookiePair = headerValue.Split(';')[0].Trim();

            var eqIndex = cookiePair.IndexOf('=');
            if (eqIndex <= 0) continue;
            var cookieName = cookiePair[..eqIndex].Trim();
            var cookieValue = cookiePair[(eqIndex + 1)..].Trim();

            if (!string.IsNullOrEmpty(cookieName))
            {
                cookieMap[cookieName] = cookieValue;
            }
        }

        return string.Join("; ", cookieMap.Select(kvp => $"{kvp.Key}={kvp.Value}"));
    }

    private static Dictionary<string, string> ParseCookieHeader(string cookieHeader)
    {
        if (string.IsNullOrWhiteSpace(cookieHeader))
            return new Dictionary<string, string>();

        return cookieHeader.Split(';')
            .Select(x => x.Split('='))
            .Where(x => x.Length == 2)
            .ToDictionary(
                x => x[0].Trim(),
                x => x[1].Trim(),
                StringComparer.OrdinalIgnoreCase
            );
    }

    private static string? GetMacFromQueryString(string queryString)
    {
        var parseQueryString = System.Web.HttpUtility.ParseQueryString(queryString);
        return parseQueryString.Get("mac");
    }
}