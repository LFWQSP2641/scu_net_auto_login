#pragma once

#include "Loginer.h"

#include <QCommandLineParser>
#include <QObject>
#include <QProcess>

class QCoreApplication;
class PlatformUtils;
class AutoTickDevice;

class SCUNetLoginApplication : public QObject
{
    Q_OBJECT

public:
    explicit SCUNetLoginApplication(QCoreApplication *app);

public slots:
    bool initialize(int argc, char *argv[]);
    void startLoginProcess();

protected slots:
    bool parseCommandLine();
    void loadConfiguration();
    bool validateConfiguration();
    void setupConnections();
    void invokeLogin();
    void startLogin(int delay);
    void outputError(const QString &error);
    void outputMessage(const QString &message);
    void outputCoreMessage(const QString &message);
    void outputFatalError(const QString &error);
    void outputHotspotMessage(const QString &message);

private slots:
    void onLoginFailed(Loginer::FailedType failedType);
    void onOpenHotspotsFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onConnectSCUNETWifiFinished(int exitCode, QProcess::ExitStatus exitStatus);

protected:
    QCoreApplication *m_app;
    QCommandLineParser m_parser;

    Loginer *m_loginer;
    PlatformUtils *m_platformUtils;
    AutoTickDevice *m_autoTickDevice;

    // 配置项
    QString m_username;
    QString m_password;
    QString m_service;
    int m_retryCount;
    int m_retryDelay;
    int m_initialDelay;
    bool m_enableHotspot;
    bool m_enableConnectSCUNETWifi;
    bool m_tryAutoTick;

    // 当前重试次数
    int m_currentRetry;

signals:
};
