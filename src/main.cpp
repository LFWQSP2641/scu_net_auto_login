#include "AutoTickDevice.h"
#include "Global.h"
#include "Loginer.h"
#include "PlatformUtils.h"
#include "Settings.h"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QFile>
#include <QMutex>
#include <QThread>
#include <QTimer>

QFile logFile("application.log");
QMutex logMutex;

void myMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context);
    QMutexLocker locker(&logMutex); // 加锁保证线程安全

    QString level;
    switch (type)
    {
    case QtDebugMsg:
        level = "DEBUG";
        break;
    case QtInfoMsg:
        level = "INFO";
        break;
    case QtWarningMsg:
        level = "WARNING";
        break;
    case QtCriticalMsg:
        level = "CRITICAL";
        break;
    case QtFatalMsg:
        level = "FATAL";
        break;
    }

    // 获取当前时间，格式为 年-月-日 时:分:秒.毫秒
    QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");

    // 获取当前线程 ID（转换为数字便于输出）
    QString threadId = QString::number(reinterpret_cast<quintptr>(QThread::currentThreadId()));

    // 构造最终的日志输出字符串
    QString formattedMessage = QString("[%1] [%2] [Thread %3] %4")
                                   .arg(currentTime, level, threadId, msg);

    // 输出到终端
    fprintf(stderr, "%s\n", formattedMessage.toLocal8Bit().constData());
    fflush(stderr);

    // 输出到文件
    if (logFile.isOpen())
    {
        QTextStream out(&logFile);
        out << formattedMessage << "\n";
        out.flush();
    }

    // 对于致命错误，终止程序
    if (type == QtFatalMsg)
    {
        abort();
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("SCU Net Auto Login");
    QCoreApplication::setApplicationVersion("1.0");

    if (!logFile.open(QIODevice::Append | QIODevice::Text))
    {
        qWarning() << "无法打开日志文件";
    }

    qInstallMessageHandler(myMessageHandler);

    Global::initOnce();

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

    // 添加重试和延迟相关的命令行参数
    QCommandLineOption retryCountOption(QStringList() << "r"
                                                      << "retry-count",
                                        "登录失败后的重试次数，-1无限重试",
                                        "count",
                                        "0");
    parser.addOption(retryCountOption);

    QCommandLineOption retryDelayOption(QStringList() << "d"
                                                      << "retry-delay",
                                        "重试之间的延迟时间(秒)",
                                        "seconds",
                                        "5");
    parser.addOption(retryDelayOption);

    QCommandLineOption initialDelayOption(QStringList() << "i"
                                                        << "initial-delay",
                                          "首次登录尝试前的延迟时间(秒)",
                                          "seconds",
                                          "0");
    parser.addOption(initialDelayOption);

    QCommandLineOption hotspotOption(QStringList() << "hotspot",
                                     "登录成功后是否开启热点");
    parser.addOption(hotspotOption);

    QCommandLineOption connectSCUNETWifiOption(QStringList() << "connect",
                                               "登录成功后是否连接SCUNET WiFi");
    parser.addOption(connectSCUNETWifiOption);

    QCommandLineOption useConfigOption(QStringList() << "c"
                                                     << "use-config",
                                       "是否使用配置文件（默认: 是）",
                                       "true/false",
                                       "true");
    parser.addOption(useConfigOption);

    // 解析命令行参数
    parser.process(a);

    const bool useConfig = parser.value(useConfigOption).toLower() == "true";

    QString username;
    QString password;
    QString service;
    int retryCount = 0;
    int retryDelay = 5000; // 默认值：5秒
    int initialDelay = 0;  // 默认值：0秒
    bool enableHotspot = false;
    bool enableConnectSCUNETWifi = false;

    // 根据 useConfig 的值加载参数
    if (useConfig)
    {
        qDebug() << "\033[1;92m[信息]\033[0m 正在从配置文件加载参数..." << Qt::endl;

        // 从配置文件加载参数
        username = parser.isSet(usernameOption) ? parser.value(usernameOption) : Settings::getSingletonSettings()->username();
        password = parser.isSet(passwordOption) ? parser.value(passwordOption) : Settings::getSingletonSettings()->password();
        service = parser.isSet(serviceOption) ? parser.value(serviceOption) : Settings::getSingletonSettings()->service();
        retryCount = parser.isSet(retryCountOption) ? parser.value(retryCountOption).toInt() : Settings::getSingletonSettings()->retryCount();
        retryDelay = parser.isSet(retryDelayOption) ? parser.value(retryDelayOption).toDouble() * 1000 : Settings::getSingletonSettings()->retryDelay();         // 转换为毫秒
        initialDelay = parser.isSet(initialDelayOption) ? parser.value(initialDelayOption).toDouble() * 1000 : Settings::getSingletonSettings()->initialDelay(); // 转换为毫秒
        enableHotspot = parser.isSet(hotspotOption) ? true : Settings::getSingletonSettings()->enableHotspot();
        enableConnectSCUNETWifi = parser.isSet(connectSCUNETWifiOption) ? true : Settings::getSingletonSettings()->enableConnectSCUNETWifi();
    }
    else
    {
        qDebug() << "\033[1;92m[信息]\033[0m 未使用配置文件，所有参数需从命令行提供" << Qt::endl;

        // 从命令行加载参数
        username = parser.value(usernameOption);
        password = parser.value(passwordOption);
        service = parser.value(serviceOption);
        retryCount = parser.value(retryCountOption).toInt();
        retryDelay = parser.value(retryDelayOption).toDouble() * 1000;     // 转换为毫秒
        initialDelay = parser.value(initialDelayOption).toDouble() * 1000; // 转换为毫秒
        enableHotspot = parser.isSet(hotspotOption);
        enableConnectSCUNETWifi = parser.isSet(connectSCUNETWifiOption);
    }

    // 检查必要参数
    if (username.isEmpty())
    {
        qWarning() << "\033[1;91m[错误]\033[0m 必须提供用户名参数 (-u, --username)" << Qt::endl;
        return 1;
    }

    if (password.isEmpty())
    {
        qWarning() << "\033[1;91m[错误]\033[0m 必须提供密码参数 (-p, --password)" << Qt::endl;
        return 1;
    }

    if (service.isEmpty())
    {
        qWarning() << "\033[1;91m[错误]\033[0m 必须提供服务类型参数 (-s, --service)" << Qt::endl;
        return 1;
    }

    // 验证服务类型
    QStringList validServices = {"CHINATELECOM", "CHINAMOBILE", "CHINAUNICOM", "EDUNET"};
    if (!validServices.contains(service))
    {
        qWarning() << "\033[1;91m[错误]\033[0m 无效的服务类型。请使用: CHINATELECOM, CHINAMOBILE, CHINAUNICOM, EDUNET" << Qt::endl;
        return 1;
    }

    Loginer loginer;
    PlatformUtils platformUtils;
    AutoTickDevice autoTickDevice;

    // 重试计数器
    int currentRetry = 0;
    bool tryAutoTick = false;

    // 连接信号处理
    QObject::connect(&loginer, &Loginer::messageReceived, [](const QString &message)
                     { qDebug() << "\033[1;92m[信息]\033[0m " << message << Qt::endl; });

    QObject::connect(&loginer, &Loginer::errorOccurred, [](const QString &error)
                     { qWarning() << "\033[1;91m[错误]\033[0m " << error << Qt::endl; });

    QObject::connect(&autoTickDevice, &AutoTickDevice::messageReceived, [&a](const QString &message)
                     { qDebug() << "\033[1;92m[信息]\033[0m " << message << Qt::endl; });

    QObject::connect(&autoTickDevice, &AutoTickDevice::errorOccurred, [&a](const QString &error)
                     { qWarning() << "\033[1;91m[错误]\033[0m " << error << Qt::endl; });

    QObject::connect(&autoTickDevice, &AutoTickDevice::coreOutput, [&a](const QString &message)
                     { qWarning() << "\033[1;92m[信息]\033[0m 核心输出: " << message << Qt::endl; });

    QObject::connect(&autoTickDevice, &AutoTickDevice::tickSuccess, &loginer, [&loginer, username, password, service]()
                     { loginer.login(username, password, service); });

    QObject::connect(&autoTickDevice, &AutoTickDevice::tickFailed, &a, [&a]()
                     { qWarning() << "\033[1;91m[错误]\033[0m 踢出设备失败" << Qt::endl; a.exit(1); });

    if (enableHotspot)
    {
        QObject::connect(&loginer, &Loginer::loginSuccess, &platformUtils, &PlatformUtils::openHotspots);
        QObject::connect(&platformUtils, &PlatformUtils::openHotspotsFinished, &a, [&a](int exitCode, QProcess::ExitStatus exitStatus)
                         {
                             if (exitStatus == QProcess::NormalExit && exitCode == 0)
                             {
                                 qDebug() << "\033[1;92m[信息]\033[0m 热点已开启" << Qt::endl;
                                 a.exit(0);
                             }
                             else
                             {
                                 qWarning() << "\033[1;91m[错误]\033[0m 开启热点失败" << Qt::endl;
                                 a.exit(1);
                             } });
    }
    else
    {
        QObject::connect(&loginer, &Loginer::loginSuccess, &a, &QCoreApplication::quit);
    }

    QObject::connect(&loginer, &Loginer::loginFailed, &loginer, [&loginer, &username, &password, &service, &currentRetry, retryCount, retryDelay, &autoTickDevice, &tryAutoTick](Loginer::FailedType failedType)
                     {
                        if (failedType == Loginer::FailedType::DeviceMaxOnline && Settings::getSingletonSettings()->enableAutoTick() && !tryAutoTick)
                        {
                            autoTickDevice.tickDevice();
                            tryAutoTick = true;
                            return;
                        }
                         // 检查是否还有重试次数
                        if (currentRetry < retryCount || retryCount == -1)
                        {
                            currentRetry++;
                            qDebug() << "\033[1;93m[重试]\033[0m 第" << currentRetry << "次重试，"
                                << retryDelay / 1000 << "秒后重试..." << Qt::endl;

                            // 延迟后重试
                            QTimer::singleShot(retryDelay, &loginer, [&loginer, username, password, service]()
                                            {
                                                qDebug() << "\033[1;93m[重试]\033[0m 正在重新尝试登录..." << Qt::endl;
                                                loginer.login(username, password, service); });
                        } });

    const auto startLogin = [&loginer, &username, &password, &service](int delay)
    {
        // 如果设置了初始延迟，则延迟后开始登录
        if (delay > 0)
        {
            qDebug() << "\033[1;92m[信息]\033[0m 将在" << delay / 1000 << "秒后开始登录..." << Qt::endl;
            QTimer::singleShot(delay, &loginer, [&loginer, username, password, service]()
                               { loginer.login(username, password, service); });
        }
        else
        {
            // 直接开始登录
            loginer.login(username, password, service);
        }
    };

    if (enableConnectSCUNETWifi)
    {
        platformUtils.connectSCUNETWifi();
        QObject::connect(&platformUtils, &PlatformUtils::connectSCUNETWifiFinished, &loginer, [&a, &platformUtils, &startLogin, initialDelay](int exitCode, QProcess::ExitStatus exitStatus)
                         {
                             if (exitStatus == QProcess::NormalExit && exitCode == 0)
                             {
                                 qDebug() << "\033[1;92m[信息]\033[0m 已连接SCUNET WiFi" << Qt::endl;
                                 startLogin(initialDelay);
                             }
                             else
                             {
                                 qWarning() << "\033[1;91m[错误]\033[0m 连接SCUNET WiFi失败" << Qt::endl;
                                 a.exit(1);
                             } });
    }
    else
    {
        startLogin(initialDelay);
    }

    return a.exec();
}
