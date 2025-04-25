#pragma once

#include <QByteArray>
#include <QString>
#include <QThread>

class RSAUtils : public QThread
{
    Q_OBJECT

public:
    explicit RSAUtils(QObject *parent = nullptr);

    static QByteArray encryptedPassword(QStringView password, QStringView mac);

public slots:
    void syncEncryptedPassword(const QString &password, const QString &mac);

protected slots:
    void run() override;

signals:
    void encryptedPasswordFinished(const QByteArray &encryptedPassword);

protected:
    QByteArray m_encryptedPassword;
    QString m_password;
    QString m_mac;
};
