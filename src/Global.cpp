#include "Global.h"

#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>

QString Global::tempDirPath = {};
QString Global::dataDirPath = {};
QString Global::configDirName = {};

void Global::initOnce()
{
#ifdef Q_OS_WIN
    Global::tempDirPath = QCoreApplication::applicationDirPath().append(QStringLiteral("/Temp"));
    Global::dataDirPath = QCoreApplication::applicationDirPath().append(QStringLiteral("/Data"));
    Global::configDirName = QCoreApplication::applicationDirPath().append(QStringLiteral("/Config"));
#else
    Global::tempDirPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    Global::dataDirPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    Global::configDirName = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
#endif

    QDir dir;
    dir.mkpath(Global::tempDirPath);
    dir.mkpath(Global::dataDirPath);
    dir.mkpath(Global::configDirName);
}

QString Global::getTempDirPath(const QString &sub)
{
    if (sub.startsWith(QStringLiteral("/")))
    {
        return Global::tempDirPath.append(sub);
    }
    else
    {
        return Global::tempDirPath.append(QStringLiteral("/")).append(sub);
    }
}

QString Global::getDataDirPath(const QString &sub)
{
    if (sub.startsWith(QStringLiteral("/")))
    {
        return Global::dataDirPath.append(sub);
    }
    else
    {
        return Global::dataDirPath.append(QStringLiteral("/")).append(sub);
    }
}

QString Global::getConfigDirPath(const QString &sub)
{
    if (sub.startsWith(QStringLiteral("/")))
    {
        return Global::configDirName.append(sub);
    }
    else
    {
        return Global::configDirName.append(QStringLiteral("/")).append(sub);
    }
}
