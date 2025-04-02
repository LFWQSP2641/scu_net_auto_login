#pragma once

#include <QObject>

class Settings : public QObject
{
    Q_OBJECT

public:
    static Settings *getSingletonSettings();
    explicit Settings(QObject *parent = nullptr);
    ~Settings();

    QString username() const;
    void setUsername(const QString &newUsername);
    QString password() const;
    void setPassword(const QString &newPassword);
    QString service() const;
    void setService(const QString &newService);
    int retryCount() const;
    void setRetryCount(int newRetryCount);
    int retryDelay() const;
    void setRetryDelay(int newRetryDelay);
    int initialDelay() const;
    void setInitialDelay(int newInitialDelay);
    bool enableHotspot() const;
    void setEnableHotspot(bool newEnableHotspot);
    bool enableConnectSCUNETWifi() const;
    void setEnableConnectSCUNETWifi(bool newEnableConnectSCUNETWifi);
    QString bypassCampusNetworkCore() const;
    void setBypassCampusNetworkCore(const QString &newBypassCampusNetworkCore);
    QStringList bypassCampusNetworkCoreCommand() const;
    void setBypassCampusNetworkCoreCommand(const QStringList &newBypassCampusNetworkCoreCommand);
    int bypassCampusNetworkCorePort() const;
    void setBypassCampusNetworkCorePort(int newBypassCampusNetworkCorePort);
    bool enableAutoTick() const;
    void setEnableAutoTick(bool newEnableAutoTick);

public slots:
    void saveSettings() const;
    void loadSettings();

protected:
    QString m_username;
    QString m_password;
    QString m_service;
    int m_retryCount;
    int m_retryDelay;
    int m_initialDelay;
    bool m_enableHotspot;
    bool m_enableConnectSCUNETWifi;

    QString m_bypassCampusNetworkCore;
    QStringList m_bypassCampusNetworkCoreCommand;
    int m_bypassCampusNetworkCorePort;

    bool m_enableAutoTick;

signals:
    void usernameChanged();
    void passwordChanged();
    void serviceChanged();
    void retryCountChanged();
    void retryDelayChanged();
    void initialDelayChanged();
    void enableHotspotChanged();
    void enableConnectSCUNETWifiChanged();
    void bypassCampusNetworkCoreChanged();
    void bypassCampusNetworkCoreCommandChanged();
    void bypassCampusNetworkCorePortChanged();
    void enableAutoTickChanged();

private:
    Q_PROPERTY(QString username READ username WRITE setUsername NOTIFY usernameChanged FINAL)
    Q_PROPERTY(QString password READ password WRITE setPassword NOTIFY passwordChanged FINAL)
    Q_PROPERTY(QString service READ service WRITE setService NOTIFY serviceChanged FINAL)
    Q_PROPERTY(int retryCount READ retryCount WRITE setRetryCount NOTIFY retryCountChanged FINAL)
    Q_PROPERTY(int retryDelay READ retryDelay WRITE setRetryDelay NOTIFY retryDelayChanged FINAL)
    Q_PROPERTY(int initialDelay READ initialDelay WRITE setInitialDelay NOTIFY initialDelayChanged FINAL)
    Q_PROPERTY(bool enableHotspot READ enableHotspot WRITE setEnableHotspot NOTIFY enableHotspotChanged FINAL)
    Q_PROPERTY(bool enableConnectSCUNETWifi READ enableConnectSCUNETWifi WRITE setEnableConnectSCUNETWifi NOTIFY enableConnectSCUNETWifiChanged FINAL)
    Q_PROPERTY(QString bypassCampusNetworkCore READ bypassCampusNetworkCore WRITE setBypassCampusNetworkCore NOTIFY bypassCampusNetworkCoreChanged FINAL)
    Q_PROPERTY(QStringList bypassCampusNetworkCoreCommand READ bypassCampusNetworkCoreCommand WRITE setBypassCampusNetworkCoreCommand NOTIFY bypassCampusNetworkCoreCommandChanged FINAL)
    Q_PROPERTY(int bypassCampusNetworkCorePort READ bypassCampusNetworkCorePort WRITE setBypassCampusNetworkCorePort NOTIFY bypassCampusNetworkCorePortChanged FINAL)
    Q_PROPERTY(bool enableAutoTick READ enableAutoTick WRITE setEnableAutoTick NOTIFY enableAutoTickChanged FINAL)
};
