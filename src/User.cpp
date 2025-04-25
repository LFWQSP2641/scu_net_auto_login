#include "User.h"

QString User::username() const
{
    return m_username;
}

void User::setUsername(const QString &newUsername)
{
    if (m_username == newUsername)
        return;
    m_username = newUsername;
}

QString User::password() const
{
    return m_password;
}

void User::setPassword(const QString &newPassword)
{
    if (m_password == newPassword)
        return;
    m_password = newPassword;
}

QString User::service() const
{
    return m_service;
}

void User::setService(const QString &newService)
{
    if (m_service == newService)
        return;
    m_service = newService;
}
