#include "Loginer.h"

#include "RSAUtils.h"

#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkCookieJar>
#include <QNetworkProxy>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QUrlQuery>

Loginer::Loginer(QObject *parent)
    : QObject(parent),
      netManager(new QNetworkAccessManager(this)),
      serviceCodeMap {
          {QStringLiteral("CHINATELECOM"), QByteArrayLiteral("%E7%94%B5%E4%BF%A1%E5%87%BA%E5%8F%A3")},
          {QStringLiteral("CHINAMOBILE"),  QByteArrayLiteral("%E7%A7%BB%E5%8A%A8%E5%87%BA%E5%8F%A3")},
          {QStringLiteral("CHINAUNICOM"),  QByteArrayLiteral("%E8%81%94%E9%80%9A%E5%87%BA%E5%8F%A3")},
          {QStringLiteral("EDUNET"),       QByteArrayLiteral("internet")                            },
},
      mainUrl(QStringLiteral("http://192.168.2.135/")),
      socket(new QTcpSocket(this)),
      tcpTimeOutTimer(new QTimer(this)),
      rsaUtils(new RSAUtils(this)),
      tcpHeaderComplete(false),
      tcpContentLength(-1),
      m_isRunning(false)
{
    netManager->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
    netManager->setProxy(QNetworkProxy::NoProxy);
    socket->setProxy(QNetworkProxy::NoProxy);

    tcpTimeOutTimer->setSingleShot(true);

    connect(this, &Loginer::firstRedirectedFinished, this, &Loginer::getRedirectInfo);
    connect(socket, &QTcpSocket::connected, this, &Loginer::onTcpSocketConnected);
    connect(socket, &QTcpSocket::readyRead, this, &Loginer::onTcpSocketReadyRead);
    connect(socket, &QTcpSocket::errorOccurred, this, &Loginer::onTcpSocketError);
    connect(socket, &QTcpSocket::disconnected, tcpTimeOutTimer, &QTimer::stop);
    connect(tcpTimeOutTimer, &QTimer::timeout, this, &Loginer::onTcpSocketTimeOut);
    connect(socket, &QTcpSocket::disconnected, this, &Loginer::onTcpSocketDisconnected);
    connect(rsaUtils, &RSAUtils::encryptedPasswordFinished, this, &Loginer::sendLoginRequest);
    connect(socket, &QTcpSocket::connected, this, &Loginer::resetTcpState);
}

Loginer::~Loginer()
{
    rsaUtils->exit(1);
    rsaUtils->wait();
}

void Loginer::login(const QString &username, const QString &password, const QString &service)
{
    m_isRunning = true;
    m_username = username.toUtf8();
    m_password = password;
    m_serviceCode = serviceCodeMap.value(service);
    if (m_serviceCode.isEmpty() && m_isRunning)
    {
        m_isRunning = false;
        emit errorOccurred(QStringLiteral("不支持的服务商: ") + service);
        emit loginFailed(FailedType::UserInputError);
        return;
    }

    emit messageReceived(QStringLiteral("正在获取登录参数..."));

    getQuery();
}

void Loginer::setRequestHeaders(QNetworkRequest &request)
{
    request.setRawHeader("Accept", "*/*");
    request.setRawHeader("Accept-Encoding", "gzip, deflate, br");
    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded; charset=UTF-8");
    request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/134.0.0.0 Safari/537.36 Edg/134.0.0.0");
}

std::optional<QString> Loginer::extractQueryString() const
{
    // 使用正则表达式提取queryString
    static const QRegularExpression regex("/index\\.jsp\\?([^'\"\n]+)");
    QRegularExpressionMatch match = regex.match(QString(tcpData));

    if (match.hasMatch())
    {
        return match.captured(1);
    }

    // 回退到原来的方法
    QString responseStr = QString(tcpData);
    int start = responseStr.indexOf("/index.jsp?") + 11;
    int end = responseStr.indexOf("'</script>");

    if (start >= 11 && end > start)
    {
        return responseStr.mid(start, end - start);
    }

    return std::nullopt;
}

void Loginer::getQuery()
{
    QNetworkRequest request {mainUrl};
    setRequestHeaders(request);
    QNetworkReply *reply = netManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]
    {
        emit this->firstRedirectedFinished(reply);
    });
}

void Loginer::sendLoginRequest(const QByteArray &encryptedPassword)
{
    if ((!query) && m_isRunning)
    {
        m_isRunning = false;
        emit errorOccurred(QStringLiteral("无法获取必要数据"));
        emit loginFailed(FailedType::RedirectError);
        return;
    }

    const QUrl loginPostUrl(QStringLiteral("eportal/InterFace.do?method=login").prepend(mainUrl));
    QNetworkRequest request {loginPostUrl};
    setRequestHeaders(request);

    QUrlQuery loginPostData;
    loginPostData.addQueryItem(QStringLiteral("userId"), m_username);
    loginPostData.addQueryItem(QStringLiteral("password"), encryptedPassword);
    loginPostData.addQueryItem(QStringLiteral("service"), m_serviceCode);
    loginPostData.addQueryItem(QStringLiteral("queryString"), query->toString());
    loginPostData.addQueryItem(QStringLiteral("operatorPwd"), {});
    loginPostData.addQueryItem(QStringLiteral("operatorUserId"), {});
    loginPostData.addQueryItem(QStringLiteral("validcode"), {});
    loginPostData.addQueryItem(QStringLiteral("passwordEncrypt"), QStringLiteral("true"));

    clearAllCookies();
    QNetworkReply *reply = netManager->post(request, loginPostData.toString(QUrl::FullyEncoded).toUtf8());
    connect(reply, &QNetworkReply::finished, this, [this, reply]
    {
        this->onLoginRequestFinished(reply);
    });

    emit messageReceived(QStringLiteral("正在登录。。。"));
}

void Loginer::resetTcpState()
{
    tcpData.clear();
    tcpHeaderComplete = false;
    tcpContentLength = -1;
}

void Loginer::clearAllCookies()
{
    netManager->cookieJar()->deleteLater();
    netManager->setCookieJar(new QNetworkCookieJar(this));
}

void Loginer::getRedirectInfo(QNetworkReply *reply)
{
    const QUrl firstUrl = reply->url();
    if (firstUrl.toString().contains(QByteArrayLiteral("success.jsp")) && m_isRunning)
    {
        m_isRunning = false;
        emit errorOccurred(QStringLiteral("已登录，无法在登录状态下获取必要数据"));
        emit loginSuccess();
        return;
    }

    reply->deleteLater();

    m_redircetUrl = firstUrl.host().toUtf8();

    socket->connectToHost(m_redircetUrl, 80);
}

void Loginer::onTcpSocketConnected()
{
    // 手动构造 HTTP 请求，确保 Host 为大写
    const QByteArray request = QByteArrayLiteral("GET / HTTP/1.1\r\n"
                                                 "Host: ")
                                   .append(m_redircetUrl)
                                   .append(QByteArrayLiteral("\r\n\r\n"));
    socket->write(request);
    tcpTimeOutTimer->start(10000);
}

void Loginer::onTcpSocketReadyRead()
{
    if (!tcpHeaderComplete)
    {
        // 读取并解析头部
        while (socket->canReadLine())
        {
            QByteArray line = socket->readLine();
            tcpData.append(line);
            if (line == "\r\n")
            { // 头部结束
                tcpHeaderComplete = true;
                // 查找 Content-Length
                QString header(tcpData);
                int pos = header.indexOf("Content-Length:", Qt::CaseInsensitive);
                if (pos != -1)
                {
                    QString lengthStr = header.mid(pos + 15, header.indexOf("\r\n", pos) - pos - 15).trimmed();
                    tcpContentLength = lengthStr.toLongLong();
                }
                break;
            }
        }
    }
    if (tcpHeaderComplete)
    {
        // 读取剩余的响应体
        tcpData.append(socket->readAll());
        emit messageReceived(QStringLiteral("[Debug] 当前数据大小:").append(QString::number(tcpData.size())).append(QStringLiteral("预期大小:")).append(QString::number(tcpContentLength)));
        // 如果有 Content-Length，判断是否读取完
        if (tcpContentLength >= 0 && tcpData.size() >= tcpContentLength)
        {
            emit messageReceived(QStringLiteral("[Debug] 数据读取完成，相应内容：").append(tcpData));
            socket->close(); // 数据读取完成，主动关闭
        }
    }
}

void Loginer::onTcpSocketError(QAbstractSocket::SocketError socketError)
{
    if ((socketError != QAbstractSocket::RemoteHostClosedError) && m_isRunning)
    {
        m_isRunning = false;
        emit errorOccurred(socket->errorString());
        emit loginFailed(FailedType::NetworkError);
    }
    socket->close();
}

void Loginer::onTcpSocketTimeOut()
{
    if (m_isRunning)
    {
        m_isRunning = false;
        emit errorOccurred(QStringLiteral("请求超时"));
        emit loginFailed(FailedType::TimeoutError);
    }
    socket->close();
}

void Loginer::onTcpSocketDisconnected()
{
    auto queryStr = extractQueryString();
    if ((!queryStr.has_value()) && m_isRunning)
    {
        m_isRunning = false;
        emit errorOccurred(QStringLiteral("无法从页面提取登录参数"));
        emit loginFailed(FailedType::Unknown);
        return;
    }

    query = QUrlQuery(queryStr.value());
    rsaUtils->syncEncryptedPassword(m_password, query->queryItemValue(QStringLiteral("mac")));
    emit messageReceived(QStringLiteral("获取登录参数成功，加密密码中..."));
#ifdef QT_DEBUG
    emit messageReceived(QStringLiteral("Debug 模式时会特别慢"));
#endif
}

void Loginer::onLoginRequestFinished(QNetworkReply *reply)
{
    const auto replyData = reply->readAll();

    if (replyData.contains(QByteArrayLiteral("\"result\":\"success\"")) && m_isRunning)
    {
        m_isRunning = false;
        emit messageReceived(QStringLiteral("登录成功"));
        emit loginSuccess();
    }
    else if (replyData.contains(QByteArrayLiteral("\"message\":\"验证码错误.\"")) && m_isRunning)
    {
        m_isRunning = false;
        emit errorOccurred(QStringLiteral("触发风控").append(replyData));
        emit loginFailed(FailedType::RiskControl);
    }
    else if (replyData.contains(QByteArrayLiteral("\"message\":\"你使用的账号已达到同时在线用户数量上限!\"")) && m_isRunning)
    {
        m_isRunning = false;
        emit errorOccurred(QStringLiteral("已达到同时在线用户数量上限").append(replyData));
        emit loginFailed(FailedType::DeviceMaxOnline);
    }
    else if (m_isRunning)
    {
        m_isRunning = false;
        emit errorOccurred(QStringLiteral("登录失败").append(replyData));
        emit loginFailed(FailedType::LoginError);
    }
    reply->deleteLater();
}
