#pragma once

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QProcess>

class QNetworkAccessManager;

class AutoTickDevice : public QObject
{
    Q_OBJECT

public:
    explicit AutoTickDevice(QObject *parent = nullptr);
    ~AutoTickDevice();

public slots:
    void tickDevice();

protected slots:
    void startByPassNetworkCore();
    void getDeviceStatus();
    void sentTickRequest();

    void setRequestHeaders(QNetworkRequest &request);
    void killByPassNetworkCore();

    void checkByPassNetworkCore();

private slots:
    void onStartByPassNetworkCoreFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onGetDeviceStatusFinished(QNetworkReply *reply);
    void onSentTickRequestFinished(QNetworkReply *reply);
    void onCheckByPassNetworkCoreFinished(QNetworkReply *reply);

protected:
    QNetworkAccessManager *netManager;
    QProcess *byPassNetworkCoreProcess;

    QString m_deviceId;
    QString m_ip;

    bool byPassNetworkSuccess = false;
    const int maxRetryCount = 10;
    const int retryDelay = 1000;
    int currentRetryCount = 0;

signals:
    void messageReceived(const QString &message);
    void errorOccurred(const QString &error);
    void coreOutput(const QString &output);
    void tickSuccess();
    void tickFailed();
};
