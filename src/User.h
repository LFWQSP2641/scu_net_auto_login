#pragma once

#include <QObject>

class Settings;

class User
{
    Q_GADGET
    friend class Settings;

public:
    User() = default;
    User(const QString &username, const QString &password, const QString &service)
        : m_username(username),
          m_password(password),
          m_service(service)
    {
    }
    User(const User &other)
        : m_username(other.m_username),
          m_password(other.m_password),
          m_service(other.m_service)
    {
    }

    bool operator==(const User &other) const
    {
        return m_username == other.m_username && m_password == other.m_password && m_service == other.m_service;
    }

    QString username() const;
    void setUsername(const QString &newUsername);

    QString password() const;
    void setPassword(const QString &newPassword);

    QString service() const;
    void setService(const QString &newService);

protected:
    QString m_username;
    QString m_password;
    QString m_service;

private:
    Q_PROPERTY(QString username READ username WRITE setUsername FINAL)
    Q_PROPERTY(QString password READ password WRITE setPassword FINAL)
    Q_PROPERTY(QString service READ service WRITE setService FINAL)
};
Q_DECLARE_METATYPE(User)
