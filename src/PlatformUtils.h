#pragma once

#include <QObject>
#include <QProcess>

class PlatformUtils : public QObject
{
    Q_OBJECT

public:
    explicit PlatformUtils(QObject *parent = nullptr);
public slots:
    void openHotspots();
    void connectSCUNETWifi();

signals:
    void errorOccurred(const QString &error);
    void openHotspotsFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void openHotspotsOutput(const QString &output);
    void connectSCUNETWifiFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void connectSCUNETWifiOutput(const QString &output);
private slots:
    void onOpenHotspotsFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onConnectSCUNETWifiFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QProcess *m_openHotspotsProcess;
    QProcess *m_connectSCUNETWifiProcess;
};
