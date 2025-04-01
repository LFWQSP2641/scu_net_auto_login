#pragma once

#include <QNetworkRequest>
#include <QObject>

class QNetworkAccessManager;

class Loginer : public QObject
{
    Q_OBJECT

public:
    explicit Loginer(QObject *parent = nullptr);

public slots:
    bool login(QStringView username, QStringView password, QStringView service);

private:
    std::optional<QUrlQuery> getQuery();
    static void setRequestHeaders(QNetworkRequest &request);

protected:
    QNetworkAccessManager *netManager;
    const QMap<QString, QByteArray> serviceCodeMap;
    const QString mainUrl;

signals:
};
