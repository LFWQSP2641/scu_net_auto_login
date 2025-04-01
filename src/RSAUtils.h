#pragma once

#include <QByteArray>
#include <QString>

class RSAUtils
{
public:
    RSAUtils() = delete;

    static QByteArray encryptedPassword(QStringView password, QStringView mac);
};
