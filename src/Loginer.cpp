#include "Loginer.h"

#include "RSAUtils.h"

#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkProxy>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QUrlQuery>

Loginer::Loginer(QObject *parent)
    : QObject {
          parent  // ？什么byd格式化
},
      netManager {new QNetworkAccessManager {this}},
      serviceCodeMap {
          {QStringLiteral("CHINATELECOM"), QByteArrayLiteral("%E7%94%B5%E4%BF%A1%E5%87%BA%E5%8F%A3")},
          {QStringLiteral("CHINAMOBILE"), QByteArrayLiteral("%E7%A7%BB%E5%8A%A8%E5%87%BA%E5%8F%A3")},
          {QStringLiteral("CHINAUNICOM"), QByteArrayLiteral("%E8%81%94%E9%80%9A%E5%87%BA%E5%8F%A3")},
          {QStringLiteral("EDUNET"), QByteArrayLiteral("internet")},
      },
      mainUrl {QStringLiteral("http://192.168.2.135/")}
{
    netManager->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
    netManager->setProxy(QNetworkProxy::NoProxy);
}

bool Loginer::login(QStringView username, QStringView password, QStringView service)
{
    const auto &serviceCode = serviceCodeMap.value(service.toString());
    if (serviceCode.isEmpty())
    {
        qDebug() << "不支持的服务商: " << service;
        return false;
    }

    const QUrl loginPostUrl(QStringLiteral("eportal/InterFace.do?method=login").prepend(mainUrl));
    QNetworkRequest request {loginPostUrl};
    setRequestHeaders(request);

    const auto query = getQuery();

    if (!query)
    {
        return false;
    }

    QUrlQuery loginPostData;
    loginPostData.addQueryItem(QStringLiteral("userId"), username.toUtf8());
    loginPostData.addQueryItem(QStringLiteral("password"), RSAUtils::encryptedPassword(password, query->queryItemValue(QStringLiteral("mac"))));
    loginPostData.addQueryItem(QStringLiteral("service"), serviceCode);
    loginPostData.addQueryItem(QStringLiteral("queryString"), query->toString());
    loginPostData.addQueryItem(QStringLiteral("operatorPwd"), {});
    loginPostData.addQueryItem(QStringLiteral("operatorUserId"), {});
    loginPostData.addQueryItem(QStringLiteral("validcode"), {});
    loginPostData.addQueryItem(QStringLiteral("passwordEncrypt"), QStringLiteral("true"));

    QNetworkReply *reply = netManager->post(request, loginPostData.toString(QUrl::FullyEncoded).toUtf8());
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    const auto &replyData = reply->readAll();
    qDebug() << replyData;

    if (replyData.contains(QByteArrayLiteral("\"result\":\"success\"")))
    {
        qDebug() << "登录成功";
        return true;
    }
    return false;
}

std::optional<QUrlQuery> Loginer::getQuery()
{
    // 第一次请求(保持不变)
    QNetworkRequest request {mainUrl};
    setRequestHeaders(request);
    QNetworkReply *reply = netManager->get(request);
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    const QUrl firstUrl = reply->url();
    if (firstUrl.toString().contains(QByteArrayLiteral("success.jsp")))
    {
        qDebug() << "已登录，无法获取必要数据";
        return std::nullopt;
    }

    reply->deleteLater();

    QTcpSocket socket;
    QByteArray data;
    socket.connectToHost(firstUrl.host(), 80);

    QObject::connect(&socket, &QTcpSocket::connected, [&socket]()
                     {
        // 手动构造 HTTP 请求，确保 Host 为大写
        QString request = "GET / HTTP/1.1\r\n"
                          "Host: 123.123.123.123\r\n"
                          "\r\n";
        socket.write(request.toUtf8()); });

    QObject::connect(&socket, &QTcpSocket::readyRead, [&socket, &data]()
                     {
        data.append(socket.readAll());
        qDebug() << "响应内容:" << data; });

    QObject::connect(&socket, &QTcpSocket::errorOccurred, [&socket](QAbstractSocket::SocketError error)
                     {
        if (error != QAbstractSocket::RemoteHostClosedError)
        {
            qDebug() << "错误:" << socket.errorString();
        }
        socket.close(); });

    connect(&socket, &QTcpSocket::disconnected, &loop, &QEventLoop::quit);
    loop.exec();

    qDebug() << "data:" << data;

    // 使用正则表达式提取queryString，更加健壮
    static const QRegularExpression regex("/index\\.jsp\\?([^'\"\n]+)");
    QRegularExpressionMatch match = regex.match(QString(data));

    std::optional<QString> queryStr;

    if (match.hasMatch())
    {
        queryStr = match.captured(1);
    }
    else
    {
        // 回退到原来的方法
        QString responseStr = QString(data);
        int start = responseStr.indexOf("/index.jsp?") + 11;
        int end = responseStr.indexOf("'</script>");

        if (start >= 11 && end > start)
        {
            queryStr = responseStr.mid(start, end - start);
        }
        else
        {
            qWarning() << "无法从页面提取登录参数";
            return std::nullopt;
        }
    }

    // 从optional<QString>转换为optional<QUrlQuery>
    if (queryStr.has_value())
    {
        return QUrlQuery(queryStr.value());
    }

    return std::nullopt;
}

void Loginer::setRequestHeaders(QNetworkRequest &request)
{
    request.setRawHeader("Accept", "*/*");
    request.setRawHeader("Accept-Encoding", "gzip, deflate, br");
    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded; charset=UTF-8");
    request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/134.0.0.0 Safari/537.36 Edg/134.0.0.0");
}
