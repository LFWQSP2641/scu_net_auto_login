#pragma once

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QTimer>
#include <QUrlQuery>

class QNetworkAccessManager;
class RSAUtils;

class Loginer : public QObject
{
    Q_OBJECT

public:
    explicit Loginer(QObject *parent = nullptr);
    ~Loginer();

public slots:
    void login(const QString &username, const QString &password, const QString &service);

protected:
    // std::optional<QUrlQuery> getQuery();
    static void setRequestHeaders(QNetworkRequest &request);

protected slots:
    void getQuery();
    void sendLoginRequest(const QByteArray &encryptedPassword);

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

    std::optional<QUrlQuery> query;

    QByteArray m_username;
    QString m_password;
    QByteArray m_serviceCode;

    RSAUtils *rsaUtils;

signals:
    void messageReceived(const QString &message);
    void errorOccurred(const QString &error);
    void firstRedirectedFinished(QNetworkReply *reply);
    void loginFinished();
};
