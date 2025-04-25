#include "AutoTickDevice.h"

#include "Settings.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkCookie>
#include <QNetworkCookieJar>
#include <QNetworkProxy>
#include <QTimer>
#include <QUrlQuery>

namespace
{
constexpr char getDeviceStatusUrl[] = "https://wfw.scu.edu.cn/netclient/wap/default/get-index";
constexpr char tickUrl[] = "https://wfw.scu.edu.cn/netclient/wap/default/offline";
} // namespace

AutoTickDevice::AutoTickDevice(QObject *parent)
    : QObject {parent},
      netManager {new QNetworkAccessManager(this)},
      byPassNetworkCoreProcess {new QProcess(this)}
{
    QNetworkProxy proxy;
    proxy.setType(Settings::getSingletonSettings()->bypassCampusNetworkSocks5Proxy() ? QNetworkProxy::Socks5Proxy : QNetworkProxy::HttpProxy);
    proxy.setHostName(QStringLiteral("127.0.0.1"));
    proxy.setPort(Settings::getSingletonSettings()->bypassCampusNetworkCorePort());
    netManager->setProxy(proxy);

    QNetworkCookie cookieUUkey;
    cookieUUkey.setName(QByteArrayLiteral("UUkey"));
    cookieUUkey.setValue(Settings::getSingletonSettings()->cookieUUkey().toUtf8());
    cookieUUkey.setDomain(QByteArrayLiteral("wfw.scu.edu.cn"));

    QNetworkCookie cookieEaiSess;
    cookieEaiSess.setName(QByteArrayLiteral("eai-sess"));
    cookieEaiSess.setValue(Settings::getSingletonSettings()->cookieEaiSess().toUtf8());
    cookieEaiSess.setDomain(QByteArrayLiteral("wfw.scu.edu.cn"));

    netManager->cookieJar()->insertCookie(cookieUUkey);
    netManager->cookieJar()->insertCookie(cookieEaiSess);

    byPassNetworkCoreProcess->setProcessChannelMode(QProcess::MergedChannels);

    connect(byPassNetworkCoreProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &AutoTickDevice::onStartByPassNetworkCoreFinished);
    connect(byPassNetworkCoreProcess, &QProcess::readyReadStandardOutput, this, [this]
    {
        emit coreOutput(byPassNetworkCoreProcess->readAllStandardOutput());
    });
}

AutoTickDevice::~AutoTickDevice()
{
    killByPassNetworkCore();
}

void AutoTickDevice::tickDevice()
{
    startByPassNetworkCore();
}

void AutoTickDevice::startByPassNetworkCore()
{
    emit messageReceived(QStringLiteral("正在启动绕过校园网认证核心..."));
    byPassNetworkCoreProcess->start(Settings::getSingletonSettings()->bypassCampusNetworkCore(), Settings::getSingletonSettings()->bypassCampusNetworkCoreCommand());
    QTimer::singleShot(1000, this, &AutoTickDevice::checkByPassNetworkCore);
}

void AutoTickDevice::getDeviceStatus()
{
    emit messageReceived(QStringLiteral("正在获取设备状态..."));
    QNetworkRequest request;
    request.setUrl(QString(getDeviceStatusUrl));
    setRequestHeaders(request);

    auto reply = netManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]
    {
        this->onGetDeviceStatusFinished(reply);
    });
}

void AutoTickDevice::sentTickRequest()
{
    emit messageReceived(QStringLiteral("正在踢出设备..."));
    QNetworkRequest request;
    request.setUrl(QString(tickUrl));
    setRequestHeaders(request);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));
    request.setRawHeader(QByteArrayLiteral("Referer"), QByteArrayLiteral(getDeviceStatusUrl));

    QUrlQuery postData;
    postData.addQueryItem(QStringLiteral("device_id"), m_deviceId);
    postData.addQueryItem(QStringLiteral("ip"), m_ip);

    auto reply = netManager->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());
    connect(reply, &QNetworkReply::finished, this, [this, reply]
    {
        this->onSentTickRequestFinished(reply);
    });
}

void AutoTickDevice::setRequestHeaders(QNetworkRequest &request)
{
    request.setRawHeader(QByteArrayLiteral("User-Agent"), QByteArrayLiteral("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/134.0.0.0 Safari/537.36 Edg/134.0.0.0"));
    request.setRawHeader(QByteArrayLiteral("Sec-Ch-Ua"), QByteArrayLiteral(R"("Chromium";v="134", "Not:A-Brand";v="24", "Microsoft Edge";v="134")"));
    request.setRawHeader(QByteArrayLiteral("Sec-Ch-Ua-Mobile"), QByteArrayLiteral("?0"));
    request.setRawHeader(QByteArrayLiteral("Sec-Ch-Ua-Platform"), QByteArrayLiteral(R"("Windows")"));
}

void AutoTickDevice::killByPassNetworkCore()
{
    byPassNetworkCoreProcess->kill();
}

void AutoTickDevice::checkByPassNetworkCore()
{
    QNetworkRequest request;
    request.setUrl(QStringLiteral("http://www.baidu.com"));
    setRequestHeaders(request);

    auto reply = netManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]
    {
        this->onCheckByPassNetworkCoreFinished(reply);
    });
}

void AutoTickDevice::onStartByPassNetworkCoreFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::NormalExit && exitCode == 0)
    {
        emit messageReceived(QStringLiteral("网络核心绕过正常关闭"));
    }
    else if (!byPassNetworkSuccess)
    {
        emit errorOccurred(QStringLiteral("启动网络核心绕过失败"));
        emit tickFailed();
    }
}

void AutoTickDevice::onGetDeviceStatusFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError)
    {
        emit errorOccurred(QStringLiteral("获取设备状态失败: ").append(reply->errorString()));
        emit tickFailed();
        return;
    }

    const auto replyData = reply->readAll();
    QJsonObject replyJson = QJsonDocument::fromJson(replyData).object();
    QJsonObject dJson = replyJson.value(QStringLiteral("d")).toObject();
    QJsonArray deviceListJson = dJson.value(QStringLiteral("list")).toArray();

    if (deviceListJson.isEmpty())
    {
        emit errorOccurred(QStringLiteral("设备状态异常: ").append(replyData));
        emit tickFailed();
        return;
    }

    // first? last?
    QJsonObject deviceJson = deviceListJson.first().toObject();
    m_deviceId = deviceJson.value(QStringLiteral("device_id")).toString();
    m_ip = deviceJson.value(QStringLiteral("ip")).toString();

    emit messageReceived(QStringLiteral("设备状态获取成功，设备ID: ").append(m_deviceId).append(QStringLiteral("IP: ")).append(m_ip));

    sentTickRequest();
}

void AutoTickDevice::onSentTickRequestFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError)
    {
        emit errorOccurred(QStringLiteral("踢出设备: ").append(reply->errorString()));
        emit tickFailed();
        return;
    }

    const auto replyData = reply->readAll();
    if (replyData.contains(QByteArrayLiteral("\"e\":0")))
    {
        emit messageReceived(QStringLiteral("踢出设备成功: ").append(replyData));
        emit tickSuccess();
        killByPassNetworkCore();
        return;
    }
    else
    {
        emit errorOccurred(QStringLiteral("踢出设备失败: ").append(replyData));
        emit tickFailed();
        return;
    }
}

void AutoTickDevice::onCheckByPassNetworkCoreFinished(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError)
    {
        emit messageReceived(QStringLiteral("网络核心绕过正常"));
        byPassNetworkSuccess = true;
        getDeviceStatus();
    }
    else
    {
        emit errorOccurred(QStringLiteral("网络核心绕过异常: ").append(reply->errorString()));
        if (currentRetryCount < maxRetryCount)
        {
            currentRetryCount++;
            QTimer::singleShot(retryDelay, this, &AutoTickDevice::checkByPassNetworkCore);
        }
        else
        {
            emit errorOccurred(QStringLiteral("网络核心绕过失败"));
            emit tickFailed();
        }
    }
}
