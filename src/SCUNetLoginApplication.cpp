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
      m_currentRetry {0},
      m_retryInProgress {false},
      m_currentUserIndex {0},
      m_hasCommandLineUser {false},
      m_tickAttempted {false}
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

    QCommandLineOption aboutOption(QStringList() << "about" << "license",
                                   "显示许可证和作者信息");
    m_parser.addOption(aboutOption);

    // 解析命令行参数
    m_parser.process(*m_app);

    if (m_parser.isSet("about") || m_parser.isSet("license"))
    {
        qInfo() << "SCU Net Auto Login";
        qInfo() << "Copyright (c) 2025 LFWQSP2641";
        qInfo() << "Author: LFWQSP2641";
        qInfo() << "License: MIT License";
        qInfo() << "This project is licensed under the MIT License, except for the Mini-GMP library,";
        qInfo() << "which is licensed under the GNU LGPL v3. See https://github.com/LFWQSP2641/scu_net_auto_login/blob/master/thirdparty/mini-gmp/COPYING.LESSERv3 for details.";
        m_app->exit(0);
        return false;
    }

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

    // 首先加载命令行参数
    m_hasCommandLineUser = m_parser.isSet("username") && m_parser.isSet("password") && m_parser.isSet("service");

    if (m_hasCommandLineUser)
    {
        // 如果命令行参数完整，创建一个用户放在列表首位
        m_username = m_parser.value("username");
        m_password = m_parser.value("password");
        m_service = m_parser.value("service");

        // 创建命令行指定的用户
        User commandLineUser;
        commandLineUser.username() = m_username;
        commandLineUser.password() = m_password;
        commandLineUser.service() = m_service;

        m_userList.clear();
        m_userList.append(commandLineUser);
    }

    if (useConfig)
    {
        qDebug() << "\033[1;92m[信息]\033[0m 正在从配置文件加载参数..." << Qt::endl;

        // 如果没有命令行用户或使用配置，添加配置文件中的用户
        if (!m_hasCommandLineUser)
        {
            // 加载用户列表
            m_userList = Settings::getSingletonSettings()->userList();

            // 如果有用户，将第一个用户设为当前用户
            if (!m_userList.isEmpty())
            {
                m_username = m_userList.first().username();
                m_password = m_userList.first().password();
                m_service = m_userList.first().service();
            }
        }

        // 加载其他配置项
        m_retryCount = m_parser.isSet("retry-count") ? m_parser.value("retry-count").toInt() : Settings::getSingletonSettings()->retryCount();
        m_retryDelay = m_parser.isSet("retry-delay") ? m_parser.value("retry-delay").toDouble() * 1000 : Settings::getSingletonSettings()->retryDelay();
        m_initialDelay = m_parser.isSet("initial-delay") ? m_parser.value("initial-delay").toDouble() * 1000 : Settings::getSingletonSettings()->initialDelay();
        m_enableHotspot = m_parser.isSet("hotspot") ? true : Settings::getSingletonSettings()->enableHotspot();
        m_enableConnectSCUNETWifi = m_parser.isSet("connect") ? true : Settings::getSingletonSettings()->enableConnectSCUNETWifi();
    }
    else
    {
        qDebug() << "\033[1;92m[信息]\033[0m 未使用配置文件，所有参数从命令行提供" << Qt::endl;

        if (!m_hasCommandLineUser)
        {
            // 如果未指定完整用户信息但明确不使用配置文件，则清空用户列表
            m_userList.clear();
        }

        // 从命令行加载其他参数
        m_retryCount = m_parser.value("retry-count").toInt();
        m_retryDelay = m_parser.value("retry-delay").toDouble() * 1000;
        m_initialDelay = m_parser.value("initial-delay").toDouble() * 1000;
        m_enableHotspot = m_parser.isSet("hotspot");
        m_enableConnectSCUNETWifi = m_parser.isSet("connect");
    }

    // 重置多用户登录状态
    m_currentUserIndex = 0;
    m_tickAttempted = false;
}

bool SCUNetLoginApplication::validateConfiguration()
{
    // 检查是否至少有一个用户
    if (m_userList.isEmpty())
    {
        outputError(QStringLiteral("未提供任何有效的用户信息"));
        return false;
    }

    // 检查每个用户配置是否有效
    const QStringList validServices = {"CHINATELECOM", "CHINAMOBILE", "CHINAUNICOM", "EDUNET"};

    return std::all_of(m_userList.cbegin(), m_userList.cend(), [this, &validServices](const User &user)
    {
        if (user.username().isEmpty())
        {
            outputError(QStringLiteral("有用户缺少用户名"));
            return false;
        }

        if (user.password().isEmpty())
        {
            outputError(QStringLiteral("用户 ") + user.username() + QStringLiteral(" 缺少密码"));
            return false;
        }

        if (user.service().isEmpty())
        {
            outputError(QStringLiteral("用户 ") + user.username() + QStringLiteral(" 缺少服务类型"));
            return false;
        }

        // 验证服务类型
        if (!validServices.contains(user.service()))
        {
            outputError(QStringLiteral("用户 ") + user.username() + QStringLiteral(" 有无效的服务类型。请使用: CHINATELECOM, CHINAMOBILE, CHINAUNICOM, EDUNET"));
            return false;
        }

        return true;
    });
}

void SCUNetLoginApplication::setupConnections()
{
    // 连接信号处理
    connect(m_loginer, &Loginer::messageReceived, this, &SCUNetLoginApplication::outputMessage);

    connect(m_loginer, &Loginer::errorOccurred, this, &SCUNetLoginApplication::outputError);

    connect(m_autoTickDevice, &AutoTickDevice::messageReceived, this, &SCUNetLoginApplication::outputMessage);

    connect(m_autoTickDevice, &AutoTickDevice::errorOccurred, this, &SCUNetLoginApplication::outputError);

    connect(m_autoTickDevice, &AutoTickDevice::coreOutput, this, &SCUNetLoginApplication::outputCoreMessage);

    connect(m_autoTickDevice, &AutoTickDevice::tickSuccess, this, [this]()
    {
        outputMessage(QStringLiteral("踢出在线设备成功，重新尝试登录..."));
        // 重置用户索引，从第一个用户重新开始尝试
        m_currentUserIndex = 0;
        invokeLogin();
    });

    connect(m_autoTickDevice, &AutoTickDevice::tickFailed, m_app, [this]()
    {
        outputFatalError(QStringLiteral("踢出设备失败"));
    });

    if (m_enableHotspot)
    {
        connect(m_loginer, &Loginer::loginSuccess, m_platformUtils, &PlatformUtils::openHotspots);
        connect(m_platformUtils, &PlatformUtils::openHotspotsFinished, this, &SCUNetLoginApplication::onOpenHotspotsFinished);
        connect(m_platformUtils, &PlatformUtils::openHotspotsOutput, this, &SCUNetLoginApplication::outputHotspotMessage);
    }
    else
    {
        connect(m_loginer, &Loginer::loginSuccess, m_app, &QCoreApplication::quit);
    }

    connect(m_loginer, &Loginer::loginFailed, this, &SCUNetLoginApplication::onLoginFailed);
}

void SCUNetLoginApplication::invokeLogin()
{
    if (m_currentUserIndex < m_userList.size())
    {
        // 使用当前用户尝试登录
        User currentUser = m_userList[m_currentUserIndex];
        outputMessage(QStringLiteral("正在尝试登录用户: ").append(currentUser.username()).append(QStringLiteral(" (")).append(currentUser.service()).append(QStringLiteral(")")));
        m_loginer->login(currentUser.username(), currentUser.password(), currentUser.service());
    }
    else
    {
        // 所有用户都已尝试登录
        if (!m_tickAttempted)
        {
            // 如果还没有尝试过踢出设备，现在尝试
            outputMessage(QStringLiteral("所有用户登录尝试都失败，尝试踢出在线设备..."));
            m_tickAttempted = true;
            m_autoTickDevice->tickDevice();
        }
        else
        {
            // 已经尝试过踢出设备，登录彻底失败
            outputFatalError(QStringLiteral("所有登录尝试均失败，包括踢出设备后的重试"));
        }
    }
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

void SCUNetLoginApplication::outputHotspotMessage(const QString &message)
{
    outputMessage(QStringLiteral("热点输出脚本: ").append(message));
}

void SCUNetLoginApplication::onLoginFailed(Loginer::FailedType failedType)
{
    // 如果已经在处理重试，则直接返回，防止重复触发
    if (m_retryInProgress)
        return;

    // 如果尚未尝试所有用户，则尝试下一个用户
    if (m_currentUserIndex < m_userList.size() - 1)
    {
        m_currentUserIndex++;
        outputMessage(QStringLiteral("登录失败，尝试下一个用户..."));
        invokeLogin();
        return;
    }

    // 已尝试最后一个用户，检查是否要尝试踢出设备
    if (failedType == Loginer::FailedType::DeviceMaxOnline &&
        Settings::getSingletonSettings()->enableAutoTick() &&
        !m_tickAttempted)
    {
        outputMessage(QStringLiteral("设备数量已达上限，尝试踢出在线设备..."));
        m_autoTickDevice->tickDevice();
        m_tickAttempted = true;
        return;
    }

    // 检查是否还有重试次数
    if (m_currentRetry < m_retryCount || m_retryCount == -1)
    {
        m_currentRetry++;
        outputMessage(QStringLiteral("登录失败，第").append(QString::number(m_currentRetry)).append(QStringLiteral("次重试，")).append(QString::number(static_cast<double>(m_retryDelay) / double(1000))).append(QStringLiteral("秒后重试...")));

        // 设置重试标志
        m_retryInProgress = true;

        // 重置用户索引，从第一个用户开始重试
        m_currentUserIndex = 0;

        // 延迟后重试
        QTimer::singleShot(m_retryDelay, this, [this]()
        {
            outputMessage(QStringLiteral("正在重新尝试登录..."));
            invokeLogin();
            // 重置重试标志
            m_retryInProgress = false;
        });
    }
}

void SCUNetLoginApplication::onOpenHotspotsFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::NormalExit && exitCode == 0)
    {
        qDebug() << "\033[1;92m[信息]\033[0m 开启热点脚本运行完成" << Qt::endl;
        m_app->exit(0);
    }
    else
    {
        qWarning() << "\033[1;91m[错误]\033[0m 无法运行热点脚本，请检查杀毒软件" << Qt::endl;
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
