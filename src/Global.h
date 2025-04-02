#pragma once

#include <QString>

class Global
{
public:
    Global() = delete;

    static void initOnce();
    static QString getTempDirPath(const QString &sub);
    static QString getDataDirPath(const QString &sub);
    static QString getConfigDirPath(const QString &sub);

protected:
    static QString tempDirPath;
    static QString dataDirPath;
    static QString configDirName;
};
