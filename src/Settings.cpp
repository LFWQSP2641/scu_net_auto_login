#include "Settings.h"

#include "Global.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

Settings *Settings::singletonSettings = nullptr;

void Settings::initOnce()
{
    singletonSettings = new Settings;
    singletonSettings->setObjectName(QStringLiteral("Settings"));
}

Settings *Settings::getSingletonSettings()
{
    return singletonSettings;
}

Settings::Settings(QObject *parent)
    : QObject {parent}
{
    loadSettings();
}

Settings::~Settings()
{
    saveSettings();
}

QList<User> Settings::userList() const
{
    return m_userList;
}

void Settings::setUserList(const QList<User> &newUserList)
{
    if (m_userList == newUserList)
        return;
    m_userList = newUserList;
    emit userListChanged();
}

int Settings::retryCount() const
{
    return m_retryCount;
}

void Settings::setRetryCount(int newRetryCount)
{
    if (m_retryCount == newRetryCount)
        return;
    m_retryCount = newRetryCount;
    emit retryCountChanged();
}

int Settings::retryDelay() const
{
    return m_retryDelay;
}

void Settings::setRetryDelay(int newRetryDelay)
{
    if (m_retryDelay == newRetryDelay)
        return;
    m_retryDelay = newRetryDelay;
    emit retryDelayChanged();
}

int Settings::initialDelay() const
{
    return m_initialDelay;
}

void Settings::setInitialDelay(int newInitialDelay)
{
    if (m_initialDelay == newInitialDelay)
        return;
    m_initialDelay = newInitialDelay;
    emit initialDelayChanged();
}

bool Settings::enableHotspot() const
{
    return m_enableHotspot;
}

void Settings::setEnableHotspot(bool newEnableHotspot)
{
    if (m_enableHotspot == newEnableHotspot)
        return;
    m_enableHotspot = newEnableHotspot;
    emit enableHotspotChanged();
}

bool Settings::enableConnectSCUNETWifi() const
{
    return m_enableConnectSCUNETWifi;
}

void Settings::setEnableConnectSCUNETWifi(bool newEnableConnectSCUNETWifi)
{
    if (m_enableConnectSCUNETWifi == newEnableConnectSCUNETWifi)
        return;
    m_enableConnectSCUNETWifi = newEnableConnectSCUNETWifi;
    emit enableConnectSCUNETWifiChanged();
}

QString Settings::bypassCampusNetworkCore() const
{
    return m_bypassCampusNetworkCore;
}

void Settings::setBypassCampusNetworkCore(const QString &newBypassCampusNetworkCore)
{
    if (m_bypassCampusNetworkCore == newBypassCampusNetworkCore)
        return;
    m_bypassCampusNetworkCore = newBypassCampusNetworkCore;
    emit bypassCampusNetworkCoreChanged();
}

QStringList Settings::bypassCampusNetworkCoreCommand() const
{
    return m_bypassCampusNetworkCoreCommand;
}

void Settings::setBypassCampusNetworkCoreCommand(const QStringList &newBypassCampusNetworkCoreCommand)
{
    if (m_bypassCampusNetworkCoreCommand == newBypassCampusNetworkCoreCommand)
        return;
    m_bypassCampusNetworkCoreCommand = newBypassCampusNetworkCoreCommand;
    emit bypassCampusNetworkCoreCommandChanged();
}

int Settings::bypassCampusNetworkCorePort() const
{
    return m_bypassCampusNetworkCorePort;
}

void Settings::setBypassCampusNetworkCorePort(int newBypassCampusNetworkCorePort)
{
    if (m_bypassCampusNetworkCorePort == newBypassCampusNetworkCorePort)
        return;
    m_bypassCampusNetworkCorePort = newBypassCampusNetworkCorePort;
    emit bypassCampusNetworkCorePortChanged();
}

bool Settings::enableAutoTick() const
{
    return m_enableAutoTick;
}

void Settings::setEnableAutoTick(bool newEnableAutoTick)
{
    if (m_enableAutoTick == newEnableAutoTick)
        return;
    m_enableAutoTick = newEnableAutoTick;
    emit enableAutoTickChanged();
}

QString Settings::cookieEaiSess() const
{
    return m_cookieEaiSess;
}

void Settings::setCookieEaiSess(const QString &newCookieEaiSess)
{
    if (m_cookieEaiSess == newCookieEaiSess)
        return;
    m_cookieEaiSess = newCookieEaiSess;
    emit cookieEaiSessChanged();
}

QString Settings::cookieUUkey() const
{
    return m_cookieUUkey;
}

void Settings::setCookieUUkey(const QString &newCookieUUkey)
{
    if (m_cookieUUkey == newCookieUUkey)
        return;
    m_cookieUUkey = newCookieUUkey;
    emit cookieUUkeyChanged();
}

bool Settings::bypassCampusNetworkSocks5Proxy() const
{
    return m_bypassCampusNetworkSocks5Proxy;
}

void Settings::setBypassCampusNetworkSocks5Proxy(bool newBypassCampusNetworkSocks5Proxy)
{
    if (m_bypassCampusNetworkSocks5Proxy == newBypassCampusNetworkSocks5Proxy)
        return;
    m_bypassCampusNetworkSocks5Proxy = newBypassCampusNetworkSocks5Proxy;
    emit bypassCampusNetworkSocks5ProxyChanged();
}

void Settings::saveSettings() const
{
    QJsonObject settingJsonObject;
    QJsonArray userListJsonArray;
    for (const auto &user : m_userList)
    {
        QJsonObject userJsonObject;
        userJsonObject.insert(QStringLiteral("username"), user.m_username);
        userJsonObject.insert(QStringLiteral("password"), user.m_password);
        userJsonObject.insert(QStringLiteral("service"), user.m_service);
        userListJsonArray.append(userJsonObject);
    }
    settingJsonObject.insert(QStringLiteral("userList"), userListJsonArray);
    settingJsonObject.insert(QStringLiteral("retryCount"), m_retryCount);
    settingJsonObject.insert(QStringLiteral("retryDelay"), m_retryDelay);
    settingJsonObject.insert(QStringLiteral("initialDelay"), m_initialDelay);
    settingJsonObject.insert(QStringLiteral("enableHotspot"), m_enableHotspot);
    settingJsonObject.insert(QStringLiteral("enableConnectSCUNETWifi"), m_enableConnectSCUNETWifi);
    settingJsonObject.insert(QStringLiteral("bypassCampusNetworkCore"), m_bypassCampusNetworkCore);
    QJsonArray bypassCampusNetworkCoreCommandList;
    for (const auto &command : m_bypassCampusNetworkCoreCommand)
    {
        bypassCampusNetworkCoreCommandList.append(command);
    }
    settingJsonObject.insert(QStringLiteral("bypassCampusNetworkCoreCommand"), bypassCampusNetworkCoreCommandList);
    settingJsonObject.insert(QStringLiteral("bypassCampusNetworkCorePort"), m_bypassCampusNetworkCorePort);
    settingJsonObject.insert(QStringLiteral("enableAutoTick"), m_enableAutoTick);
    settingJsonObject.insert(QStringLiteral("cookieUUkey"), m_cookieUUkey);
    settingJsonObject.insert(QStringLiteral("cookieEaiSess"), m_cookieEaiSess);
    settingJsonObject.insert(QStringLiteral("bypassCampusNetworkSocks5Proxy"), m_bypassCampusNetworkSocks5Proxy);

    QFile file {Global::getConfigDirPath(QStringLiteral("/setting.json"))};
    file.open(QFile::WriteOnly);
    file.write(QJsonDocument(settingJsonObject).toJson());
    file.close();
}

void Settings::loadSettings()
{
    QByteArray fileData;
    QFile file {Global::getConfigDirPath(QStringLiteral("/setting.json"))};
    if (file.exists())
    {
        file.open(QFile::ReadOnly);
        fileData = file.readAll();
    }
    else
    {
        file.open(QFile::NewOnly);
    }
    file.close();

    const auto settingJsonObject {QJsonDocument::fromJson(fileData).object()};

    const auto userListJsonArray {settingJsonObject.value(QStringLiteral("userList")).toArray()};
    for (const auto &userJsonObject : userListJsonArray)
    {
        User user;
        const auto userObject {userJsonObject.toObject()};
        user.m_username = userObject.value(QStringLiteral("username")).toString();
        user.m_password = userObject.value(QStringLiteral("password")).toString();
        user.m_service = userObject.value(QStringLiteral("service")).toString();
        m_userList.append(user);
    }

    if (settingJsonObject.contains(QStringLiteral("username")))
    {
        User user;
        user.m_username = settingJsonObject.value(QStringLiteral("username")).toString();
        user.m_password = settingJsonObject.value(QStringLiteral("password")).toString();
        user.m_service = settingJsonObject.value(QStringLiteral("service")).toString();
        m_userList.append(user);
    }
    m_retryCount = settingJsonObject.value(QStringLiteral("retryCount")).toInt(0);
    m_retryDelay = settingJsonObject.value(QStringLiteral("retryDelay")).toInt(5);
    m_initialDelay = settingJsonObject.value(QStringLiteral("initialDelay")).toInt(0);
    m_enableHotspot = settingJsonObject.value(QStringLiteral("enableHotspot")).toBool(false);
    m_enableConnectSCUNETWifi = settingJsonObject.value(QStringLiteral("enableConnectSCUNETWifi")).toBool(false);
    m_bypassCampusNetworkCore = settingJsonObject.value(QStringLiteral("bypassCampusNetworkCore")).toString();
    const auto bypassCampusNetworkCoreCommandList {settingJsonObject.value(QStringLiteral("bypassCampusNetworkCoreCommand")).toArray()};
    for (const auto &command : bypassCampusNetworkCoreCommandList)
    {
        m_bypassCampusNetworkCoreCommand.append(command.toString());
    }
    m_bypassCampusNetworkCorePort = settingJsonObject.value(QStringLiteral("bypassCampusNetworkCorePort")).toInt(0);
    m_enableAutoTick = settingJsonObject.value(QStringLiteral("enableAutoTick")).toBool(false);
    m_cookieUUkey = settingJsonObject.value(QStringLiteral("cookieUUkey")).toString();
    m_cookieEaiSess = settingJsonObject.value(QStringLiteral("cookieEaiSess")).toString();
    m_bypassCampusNetworkSocks5Proxy = settingJsonObject.value(QStringLiteral("bypassCampusNetworkSocks5Proxy")).toBool(true);
}
