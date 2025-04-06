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
    void getWifiInterface();

signals:
    void errorOccurred(const QString &error);
    void openHotspotsFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void openHotspotsOutput(const QString &output);
    void connectSCUNETWifiFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void connectSCUNETWifiOutput(const QString &output);
    void wifiInterfaceFound(const QString &interface);
private slots:
    void onOpenHotspotsFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onConnectSCUNETWifiFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onGetWifiInterfaceFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void processWifiInterfaceOutput();

private:
    QProcess *m_openHotspotsProcess;
    QProcess *m_connectSCUNETWifiProcess;
    QProcess *m_getWifiInterfaceProcess;

    enum class WifiInterfaceStep
    {
        None,
        Windows,
        LinuxNMCLI,
        LinuxIWConfig,
        LinuxIP,
        MacHardwarePorts,
        MacNetworkServices
    };
    WifiInterfaceStep m_wifiInterfaceStep;
    QString m_foundInterface;
    bool m_interfaceSignalEmitted;
};
