#pragma once

#include "User.h"

#include <QObject>

class Settings : public QObject
{
    Q_OBJECT

public:
    static void initOnce();
    static Settings *getSingletonSettings();
    ~Settings();

    QList<User> userList() const;
    void setUserList(const QList<User> &newUserList);
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
    QString cookieUUkey() const;
    void setCookieUUkey(const QString &newCookieUUkey);
    QString cookieEaiSess() const;
    void setCookieEaiSess(const QString &newCookieEaiSess);
    bool bypassCampusNetworkSocks5Proxy() const;
    void setBypassCampusNetworkSocks5Proxy(bool newBypassCampusNetworkSocks5Proxy);

public slots:
    void saveSettings() const;
    void loadSettings();

protected:
    static Settings *singletonSettings;
    explicit Settings(QObject *parent = nullptr);

    QList<User> m_userList;
    int m_retryCount;
    int m_retryDelay;
    int m_initialDelay;
    bool m_enableHotspot;
    bool m_enableConnectSCUNETWifi;

    QString m_bypassCampusNetworkCore;
    QStringList m_bypassCampusNetworkCoreCommand;
    int m_bypassCampusNetworkCorePort;

    bool m_enableAutoTick;

    QString m_cookieUUkey;
    QString m_cookieEaiSess;

    bool m_bypassCampusNetworkSocks5Proxy;

signals:
    void userListChanged();
    void retryCountChanged();
    void retryDelayChanged();
    void initialDelayChanged();
    void enableHotspotChanged();
    void enableConnectSCUNETWifiChanged();
    void bypassCampusNetworkCoreChanged();
    void bypassCampusNetworkCoreCommandChanged();
    void bypassCampusNetworkCorePortChanged();
    void enableAutoTickChanged();
    void cookieUUkeyChanged();
    void cookieEaiSessChanged();
    void bypassCampusNetworkSocks5ProxyChanged();

private:
    Q_PROPERTY(QList<User> userList READ userList WRITE setUserList NOTIFY userListChanged FINAL)
    Q_PROPERTY(int retryCount READ retryCount WRITE setRetryCount NOTIFY retryCountChanged FINAL)
    Q_PROPERTY(int retryDelay READ retryDelay WRITE setRetryDelay NOTIFY retryDelayChanged FINAL)
    Q_PROPERTY(int initialDelay READ initialDelay WRITE setInitialDelay NOTIFY initialDelayChanged FINAL)
    Q_PROPERTY(bool enableHotspot READ enableHotspot WRITE setEnableHotspot NOTIFY enableHotspotChanged FINAL)
    Q_PROPERTY(bool enableConnectSCUNETWifi READ enableConnectSCUNETWifi WRITE setEnableConnectSCUNETWifi NOTIFY enableConnectSCUNETWifiChanged FINAL)
    Q_PROPERTY(QString bypassCampusNetworkCore READ bypassCampusNetworkCore WRITE setBypassCampusNetworkCore NOTIFY bypassCampusNetworkCoreChanged FINAL)
    Q_PROPERTY(QStringList bypassCampusNetworkCoreCommand READ bypassCampusNetworkCoreCommand WRITE setBypassCampusNetworkCoreCommand NOTIFY bypassCampusNetworkCoreCommandChanged FINAL)
    Q_PROPERTY(int bypassCampusNetworkCorePort READ bypassCampusNetworkCorePort WRITE setBypassCampusNetworkCorePort NOTIFY bypassCampusNetworkCorePortChanged FINAL)
    Q_PROPERTY(bool enableAutoTick READ enableAutoTick WRITE setEnableAutoTick NOTIFY enableAutoTickChanged FINAL)
    Q_PROPERTY(QString cookieUUkey READ cookieUUkey WRITE setCookieUUkey NOTIFY cookieUUkeyChanged FINAL)
    Q_PROPERTY(QString cookieEaiSess READ cookieEaiSess WRITE setCookieEaiSess NOTIFY cookieEaiSessChanged FINAL)
    Q_PROPERTY(bool bypassCampusNetworkSocks5Proxy READ bypassCampusNetworkSocks5Proxy WRITE setBypassCampusNetworkSocks5Proxy NOTIFY bypassCampusNetworkSocks5ProxyChanged FINAL)
};
