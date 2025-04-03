#include "SCUNetLoginApplication.h"

#include "AutoTickDevice.h"
#include "PlatformUtils.h"
#include "Settings.h"

#include <QCoreApplication>

SCUNetLoginApplication::SCUNetLoginApplication(QCoreApplication *app)
    : QObject {app},
      m_app {app},
      m_loginer {new Loginer(this)},
      m_platformUtils {new PlatformUtils(this)},
      m_autoTickDevice {new AutoTickDevice(this)},
      m_retryCount {0},
      m_retryDelay {5},
      m_initialDelay {0},
      m_enableHotspot {false},
      m_enableConnectSCUNETWifi {false},
      m_tryAutoTick {false},
      m_currentRetry {0}
{
}

bool SCUNetLoginApplication::initialize(int argc, char *argv[])
{
    // 设置应用程序信息
    QCoreApplication::setApplicationName("SCU Net Auto Login");
    QCoreApplication::setApplicationVersion("1.0");

    // 初始化解析器
    m_parser.setApplicationDescription("四川大学校园网自动登录工具");
    m_parser.addHelpOption();
    m_parser.addVersionOption();

    // 添加命令行参数
    QCommandLineOption usernameOption(QStringList() << "u"
                                                    << "username",
                                      "校园网账号",
                                      "username");
    m_parser.addOption(usernameOption);

    QCommandLineOption passwordOption(QStringList() << "p"
                                                    << "password",
                                      "校园网密码",
                                      "password");
    m_parser.addOption(passwordOption);

    QCommandLineOption serviceOption(QStringList() << "s"
                                                   << "service",
                                     "网络服务类型 (CHINATELECOM, CHINAMOBILE, CHINAUNICOM, EDUNET)",
                                     "service");
    m_parser.addOption(serviceOption);

    // 添加重试和延迟相关的命令行参数
    QCommandLineOption retryCountOption(QStringList() << "r"
                                                      << "retry-count",
                                        "登录失败后的重试次数，-1无限重试",
                                        "count",
                                        "0");
    m_parser.addOption(retryCountOption);

    QCommandLineOption retryDelayOption(QStringList() << "d"
                                                      << "retry-delay",
                                        "重试之间的延迟时间(秒)",
                                        "seconds",
                                        "5");
    m_parser.addOption(retryDelayOption);

    QCommandLineOption initialDelayOption(QStringList() << "i"
                                                        << "initial-delay",
                                          "首次登录尝试前的延迟时间(秒)",
                                          "seconds",
                                          "0");
    m_parser.addOption(initialDelayOption);

    QCommandLineOption hotspotOption(QStringList() << "hotspot", "登录成功后是否开启热点");
    m_parser.addOption(hotspotOption);

    QCommandLineOption connectSCUNETWifiOption(QStringList() << "connect", "登录成功后是否连接SCUNET WiFi");
    m_parser.addOption(connectSCUNETWifiOption);

    QCommandLineOption useConfigOption(QStringList() << "c"
                                                     << "use-config",
                                       "是否使用配置文件（默认: 是）",
                                       "true/false",
                                       "true");
    m_parser.addOption(useConfigOption);

    // 解析命令行参数
    m_parser.process(*m_app);

    if (!parseCommandLine())
    {
        return false;
    }

    setupConnections();
    return true;
}

void SCUNetLoginApplication::startLoginProcess()
{
    if (m_enableConnectSCUNETWifi)
    {
        m_platformUtils->connectSCUNETWifi();
        connect(m_platformUtils, &PlatformUtils::connectSCUNETWifiFinished, this, &SCUNetLoginApplication::onConnectSCUNETWifiFinished);
    }
    else
    {
        startLogin(m_initialDelay);
    }
}

bool SCUNetLoginApplication::parseCommandLine()
{
    loadConfiguration();
    return validateConfiguration();
}

void SCUNetLoginApplication::loadConfiguration()
{
    const bool useConfig = m_parser.value("use-config").toLower() == "true";

    if (useConfig)
    {
        qDebug() << "\033[1;92m[信息]\033[0m 正在从配置文件加载参数..." << Qt::endl;

        // 从配置文件加载参数
        m_username = m_parser.isSet("username") ? m_parser.value("username") : Settings::getSingletonSettings()->username();
        m_password = m_parser.isSet("password") ? m_parser.value("password") : Settings::getSingletonSettings()->password();
        m_service = m_parser.isSet("service") ? m_parser.value("service") : Settings::getSingletonSettings()->service();
        m_retryCount = m_parser.isSet("retry-count") ? m_parser.value("retry-count").toInt() : Settings::getSingletonSettings()->retryCount();
        m_retryDelay = m_parser.isSet("retry-delay") ? m_parser.value("retry-delay").toDouble() * 1000 : Settings::getSingletonSettings()->retryDelay();
        m_initialDelay = m_parser.isSet("initial-delay") ? m_parser.value("initial-delay").toDouble() * 1000 : Settings::getSingletonSettings()->initialDelay();
        m_enableHotspot = m_parser.isSet("hotspot") ? true : Settings::getSingletonSettings()->enableHotspot();
        m_enableConnectSCUNETWifi = m_parser.isSet("connect") ? true : Settings::getSingletonSettings()->enableConnectSCUNETWifi();
    }
    else
    {
        qDebug() << "\033[1;92m[信息]\033[0m 未使用配置文件，所有参数需从命令行提供" << Qt::endl;

        // 从命令行加载参数
        m_username = m_parser.value("username");
        m_password = m_parser.value("password");
        m_service = m_parser.value("service");
        m_retryCount = m_parser.value("retry-count").toInt();
        m_retryDelay = m_parser.value("retry-delay").toDouble() * 1000;
        m_initialDelay = m_parser.value("initial-delay").toDouble() * 1000;
        m_enableHotspot = m_parser.isSet("hotspot");
        m_enableConnectSCUNETWifi = m_parser.isSet("connect");
    }
}

bool SCUNetLoginApplication::validateConfiguration()
{
    // 检查必要参数
    if (m_username.isEmpty())
    {
        outputError(QStringLiteral("必须提供用户名参数 (-u, --username)"));
        return false;
    }

    if (m_password.isEmpty())
    {
        outputError(QStringLiteral("必须提供密码参数 (-p, --password)"));
        return false;
    }

    if (m_service.isEmpty())
    {
        outputError(QStringLiteral("必须提供服务类型参数 (-s, --service)"));
        return false;
    }

    // 验证服务类型
    QStringList validServices = {"CHINATELECOM", "CHINAMOBILE", "CHINAUNICOM", "EDUNET"};
    if (!validServices.contains(m_service))
    {
        outputError(QStringLiteral("无效的服务类型。请使用: CHINATELECOM, CHINAMOBILE, CHINAUNICOM, EDUNET"));
        return false;
    }

    return true;
}

void SCUNetLoginApplication::setupConnections()
{
    // 连接信号处理
    connect(m_loginer, &Loginer::messageReceived, this, &SCUNetLoginApplication::outputMessage);

    connect(m_loginer, &Loginer::errorOccurred, this, &SCUNetLoginApplication::outputError);

    connect(m_autoTickDevice, &AutoTickDevice::messageReceived, this, &SCUNetLoginApplication::outputMessage);

    connect(m_autoTickDevice, &AutoTickDevice::errorOccurred, this, &SCUNetLoginApplication::outputError);

    connect(m_autoTickDevice, &AutoTickDevice::coreOutput, this, &SCUNetLoginApplication::outputCoreMessage);

    connect(m_autoTickDevice, &AutoTickDevice::tickSuccess, this, &SCUNetLoginApplication::invokeLogin);

    connect(m_autoTickDevice, &AutoTickDevice::tickFailed, m_app, [this]()
            { outputFatalError(QStringLiteral("踢出设备失败")); });

    if (m_enableHotspot)
    {
        connect(m_loginer, &Loginer::loginSuccess, m_platformUtils, &PlatformUtils::openHotspots);
        connect(m_platformUtils, &PlatformUtils::openHotspotsFinished, this, &SCUNetLoginApplication::onOpenHotspotsFinished);
    }
    else
    {
        connect(m_loginer, &Loginer::loginSuccess, m_app, &QCoreApplication::quit);
    }

    connect(m_loginer, &Loginer::loginFailed, this, &SCUNetLoginApplication::onLoginFailed);
}

void SCUNetLoginApplication::invokeLogin()
{
    m_loginer->login(m_username, m_password, m_service);
}

void SCUNetLoginApplication::startLogin(int delay)
{
    // 如果设置了初始延迟，则延迟后开始登录
    if (delay > 0)
    {
        qDebug() << "\033[1;92m[信息]\033[0m 将在" << delay / 1000 << "秒后开始登录..." << Qt::endl;
        QTimer::singleShot(delay, this, &SCUNetLoginApplication::invokeLogin);
    }
    else
    {
        // 直接开始登录
        invokeLogin();
    }
}

void SCUNetLoginApplication::outputError(const QString &error)
{
    qWarning() << "\033[1;91m[错误]\033[0m " << error << Qt::endl;
}

void SCUNetLoginApplication::outputMessage(const QString &message)
{
    qDebug() << "\033[1;92m[信息]\033[0m " << message << Qt::endl;
}

void SCUNetLoginApplication::outputCoreMessage(const QString &message)
{
    qDebug() << "\033[1;92m[信息]\033[0m 核心输出: " << message << Qt::endl;
}

void SCUNetLoginApplication::outputFatalError(const QString &error)
{
    qCritical() << "\033[1;91m[致命错误]\033[0m " << error << Qt::endl;
    m_app->exit(1);
}

void SCUNetLoginApplication::onLoginFailed(Loginer::FailedType failedType)
{
    if (failedType == Loginer::FailedType::DeviceMaxOnline && Settings::getSingletonSettings()->enableAutoTick() && !m_tryAutoTick)
    {
        m_autoTickDevice->tickDevice();
        m_tryAutoTick = true;
        return;
    }

    // 检查是否还有重试次数
    if (m_currentRetry < m_retryCount || m_retryCount == -1)
    {
        m_currentRetry++;
        outputMessage(QStringLiteral("登录失败，第").append(QString::number(m_currentRetry)).append(QStringLiteral("次重试，")).append(QString::number(static_cast<double>(m_retryDelay) / double(1000))).append(QStringLiteral("秒后重试...")));
        // 延迟后重试
        QTimer::singleShot(m_retryDelay, this, [this]()
                           {
                               outputMessage(QStringLiteral("正在重新尝试登录..."));
                               invokeLogin(); });
    }
}

void SCUNetLoginApplication::onOpenHotspotsFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::NormalExit && exitCode == 0)
    {
        qDebug() << "\033[1;92m[信息]\033[0m 热点已开启" << Qt::endl;
        m_app->exit(0);
    }
    else
    {
        qWarning() << "\033[1;91m[错误]\033[0m 开启热点失败" << Qt::endl;
        m_app->exit(1);
    }
}

void SCUNetLoginApplication::onConnectSCUNETWifiFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::NormalExit && exitCode == 0)
    {
        outputMessage("SCUNET WiFi连接成功");
        startLogin(m_initialDelay);
    }
    else
    {
        outputError("SCUNET WiFi连接失败");
        m_app->exit(1);
    }
}
