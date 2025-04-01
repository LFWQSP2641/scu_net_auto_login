#include "Loginer.h"
#include "RSAUtils.h"

#include <QCommandLineParser>
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("SCU Net Auto Login");
    QCoreApplication::setApplicationVersion("1.0");

    // 创建命令行参数解析器
    QCommandLineParser parser;
    parser.setApplicationDescription("四川大学校园网自动登录工具");
    parser.addHelpOption();
    parser.addVersionOption();

    // 添加命令行参数
    QCommandLineOption usernameOption(QStringList() << "u"
                                                    << "username",
                                      "校园网账号",
                                      "username");
    parser.addOption(usernameOption);

    QCommandLineOption passwordOption(QStringList() << "p"
                                                    << "password",
                                      "校园网密码",
                                      "password");
    parser.addOption(passwordOption);

    QCommandLineOption serviceOption(QStringList() << "s"
                                                   << "service",
                                     "网络服务类型 (CHINATELECOM, CHINAMOBILE, CHINAUNICOM, EDUNET)",
                                     "service");
    parser.addOption(serviceOption);

    // 解析命令行参数
    parser.process(a);

    // 检查必要参数
    if (!parser.isSet(usernameOption))
    {
        qWarning() << "\033[1;91m[错误]\033[0m 必须提供用户名参数 (-u, --username)" << Qt::endl;
        return 1;
    }

    if (!parser.isSet(passwordOption))
    {
        qWarning() << "\033[1;91m[错误]\033[0m 必须提供密码参数 (-p, --password)" << Qt::endl;
        return 1;
    }

    if (!parser.isSet(serviceOption))
    {
        qWarning() << "\033[1;91m[错误]\033[0m 必须提供服务类型参数 (-s, --service)" << Qt::endl;
        return 1;
    }

    // 获取参数值
    QString username = parser.value(usernameOption);
    QString password = parser.value(passwordOption);
    QString service = parser.value(serviceOption);

    // 验证服务类型
    QStringList validServices = {"CHINATELECOM", "CHINAMOBILE", "CHINAUNICOM", "EDUNET"};
    if (!validServices.contains(service))
    {
        qWarning() << "\033[1;91m[错误]\033[0m 无效的服务类型。请使用: CHINATELECOM, CHINAMOBILE, CHINAUNICOM, EDUNET" << Qt::endl;
        return 1;
    }

    Loginer loginer;
    loginer.login(username, password, service);

    QObject::connect(&loginer, &Loginer::messageReceived, [](const QString &message)
                     { qDebug() << "\033[1;92m[信息]\033[0m " << message << Qt::endl; });
    QObject::connect(&loginer, &Loginer::errorOccurred, [](const QString &error)
                     { qWarning() << "\033[1;91m[错误]\033[0m " << error << Qt::endl; });
    QObject::connect(&loginer, &Loginer::loginFinished, &a, &QCoreApplication::quit);

    return a.exec();
}
