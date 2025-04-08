#pragma once

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QUrlQuery>
#include <optional>

class QNetworkAccessManager;
class RSAUtils;

class Loginer : public QObject
{
    Q_OBJECT

public:
    explicit Loginer(QObject *parent = nullptr);
    ~Loginer();

    enum FailedType
    {
        NetworkError,
        RedirectError,
        LoginError,
        TimeoutError,
        RiskControl,
        DeviceMaxOnline,
        UserInputError,
        Unknown
    };
    Q_ENUM(FailedType)

public slots:
    void login(const QString &username, const QString &password, const QString &service);

protected:
    // std::optional<QUrlQuery> getQuery();
    static void setRequestHeaders(QNetworkRequest &request);
    std::optional<QString> extractQueryString() const;

protected slots:
    void getQuery();
    void sendLoginRequest(const QByteArray &encryptedPassword);
    void resetTcpState();
    void clearAllCookies();

private slots:
    void getRedirectInfo(QNetworkReply *reply);
    void onTcpSocketConnected();
    void onTcpSocketReadyRead();
    void onTcpSocketError(QAbstractSocket::SocketError socketError);
    void onTcpSocketTimeOut();
    void onTcpSocketDisconnected();
    void onLoginRequestFinished(QNetworkReply *reply);

protected:
    QNetworkAccessManager *netManager;
    const QMap<QString, QByteArray> serviceCodeMap;
    const QString mainUrl;

    QTcpSocket *socket;
    QByteArray tcpData;
    bool tcpHeaderComplete;
    qint64 tcpContentLength = -1;
    QTimer *tcpTimeOutTimer;
    QByteArray m_redircetUrl;

    std::optional<QUrlQuery> query;

    QByteArray m_username;
    QString m_password;
    QByteArray m_serviceCode;

    RSAUtils *rsaUtils;

    bool m_isRunning;

signals:
    void messageReceived(const QString &message);
    void errorOccurred(const QString &error);
    void firstRedirectedFinished(QNetworkReply *reply);
    void loginFailed(Loginer::FailedType failedType);
    void loginSuccess();
};
