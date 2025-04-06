#include "PlatformUtils.h"

#include <QFile>
#include <QProcess>
#include <QRegularExpression>

namespace
{
#ifdef Q_OS_WIN
// Julius Hardt created this script to toggle the Mobile Hotspot on Windows 10/11.
// https://stackoverflow.com/users/7006729/julius-hardt
// https://stackoverflow.com/a/55563418
// https://gist.github.com/LFWQSP2641/0de8d6db8c00e68f562ea5042f707a2c
constexpr char windows10HotspotPowerShellScript[] = R"E(
[Windows.System.UserProfile.LockScreen, Windows.System.UserProfile, ContentType = WindowsRuntime] | Out-Null
Add-Type -AssemblyName System.Runtime.WindowsRuntime
$asTaskGeneric = ([System.WindowsRuntimeSystemExtensions].GetMethods() | Where-Object {
        $_.Name -eq 'AsTask' -and $_.GetParameters().Count -eq 1 -and $_.GetParameters()[0].ParameterType.Name -eq 'IAsyncOperation`1'
    })[0]
Function Await($WinRtTask, $ResultType, $TimeoutSeconds = 30) {
    $asTask = $asTaskGeneric.MakeGenericMethod($ResultType)
    $netTask = $asTask.Invoke($null, @($WinRtTask))
    if (-not $netTask.Wait($TimeoutSeconds * 1000)) {
        Write-Error "Operation timed out (waited $TimeoutSeconds seconds)."
        return $null
    }
    return $netTask.Result
}
Function AwaitAction($WinRtAction, $TimeoutSeconds = 30) {
    $asTask = ([System.WindowsRuntimeSystemExtensions].GetMethods() | Where-Object {
            $_.Name -eq 'AsTask' -and $_.GetParameters().Count -eq 1 -and -not $_.IsGenericMethod
        })[0]
    $netTask = $asTask.Invoke($null, @($WinRtAction))
    if (-not $netTask.Wait($TimeoutSeconds * 1000)) {
        Write-Error "Operation timed out (waited $TimeoutSeconds seconds)."
        return $false
    }
    return $true
}
$connectionProfile = [Windows.Networking.Connectivity.NetworkInformation, Windows.Networking.Connectivity, ContentType = WindowsRuntime]::GetInternetConnectionProfile()
$tetheringManager = [Windows.Networking.NetworkOperators.NetworkOperatorTetheringManager, Windows.Networking.NetworkOperators, ContentType = WindowsRuntime]::CreateFromConnectionProfile($connectionProfile)
$state = $tetheringManager.TetheringOperationalState.ToString()
Write-Output "Current Mobile Hotspot status: $state"
switch ($state) {
    "Off" {
        # Hotspot is off, try to turn it on
        $result = Await ($tetheringManager.StartTetheringAsync()) ([Windows.Networking.NetworkOperators.NetworkOperatorTetheringOperationResult, Windows.Networking.NetworkOperators, ContentType = WindowsRuntime])
        if ($result -and ($result.Status.ToString() -eq "Success")) {
            Write-Output "Mobile Hotspot successfully turned on."
        }
        else {
            Write-Error "Failed to start Mobile Hotspot, status: $($result.Status)"
        }
    }
    "On" {
        Write-Output "Mobile Hotspot is already on. No action needed."
    }
    default {
        Write-Output "Mobile Hotspot is in a transitional state (status code: $state). Please try again later."
    }
}
)E";
#endif
#ifdef Q_OS_MAC
constexpr char macHotspotScript[] = R"(
do shell script "sudo launchctl load -w /System/Library/LaunchDaemons/com.apple.InternetSharing.plist" with administrator privileges
)";
#endif
} // namespace

PlatformUtils::PlatformUtils(QObject *parent)
    : QObject {parent},
      m_openHotspotsProcess {nullptr},
      m_connectSCUNETWifiProcess {nullptr},
      m_getWifiInterfaceProcess {nullptr},
      m_wifiInterfaceStep {WifiInterfaceStep::None},
      m_interfaceSignalEmitted {false}
{
}

void PlatformUtils::openHotspots()
{
    m_openHotspotsProcess = new QProcess(this);
    m_openHotspotsProcess->setProcessChannelMode(QProcess::MergedChannels);
#if defined(Q_OS_WIN)
    m_openHotspotsProcess->start("powershell", QStringList() << "-Command" << windows10HotspotPowerShellScript);
#elif defined(Q_OS_MAC)
    m_openHotspotsProcess->start("osascript", QStringList() << "-e" << macHotspotScript);
#endif
    connect(m_openHotspotsProcess, &QProcess::finished, this, &PlatformUtils::onOpenHotspotsFinished);
    connect(m_openHotspotsProcess, &QProcess::readyReadStandardOutput, this, [this]
            { emit openHotspotsOutput(m_openHotspotsProcess->readAllStandardOutput()); });
}

void PlatformUtils::connectSCUNETWifi()
{
    // 首先获取WiFi接口
#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
    // 只有在Mac和Linux下才需要获取接口
    connect(this, &PlatformUtils::wifiInterfaceFound, this, [this](const QString &interface)
            {
        // 断开连接，避免多次调用
        disconnect(this, &PlatformUtils::wifiInterfaceFound, this, nullptr);

        m_connectSCUNETWifiProcess = new QProcess(this);
        m_connectSCUNETWifiProcess->setProcessChannelMode(QProcess::MergedChannels);

#if defined(Q_OS_LINUX)
        if (!interface.isEmpty()) {
            m_connectSCUNETWifiProcess->start("nmcli", QStringList() << "device"
                                                                     << "wifi"
                                                                     << "connect"
                                                                     << "SCUNET"
                                                                     << "ifname"
                                                                     << interface);
        } else {
            m_connectSCUNETWifiProcess->start("nmcli", QStringList() << "device"
                                                                     << "wifi"
                                                                     << "connect"
                                                                     << "SCUNET");
        }
#elif defined(Q_OS_MAC)
        QString wifiInterface = interface.isEmpty() ? "en0" : interface;
        m_connectSCUNETWifiProcess->start("networksetup", QStringList() << "-setairportnetwork"
                                                                        << wifiInterface
                                                                        << "SCUNET");
#endif
        connect(m_connectSCUNETWifiProcess, &QProcess::finished, this, &PlatformUtils::onConnectSCUNETWifiFinished);
        connect(m_connectSCUNETWifiProcess, &QProcess::readyReadStandardOutput, this, [this]
                { emit connectSCUNETWifiOutput(m_connectSCUNETWifiProcess->readAllStandardOutput()); }); });

    // 开始异步获取WiFi接口
    getWifiInterface();
#else
    // 对于Windows，直接连接
    m_connectSCUNETWifiProcess = new QProcess(this);
    m_connectSCUNETWifiProcess->setProcessChannelMode(QProcess::MergedChannels);
    m_connectSCUNETWifiProcess->start("netsh", QStringList() << "wlan"
                                                             << "connect"
                                                             << "name=\"SCUNET\"");
    connect(m_connectSCUNETWifiProcess, &QProcess::finished, this, &PlatformUtils::onConnectSCUNETWifiFinished);
    connect(m_connectSCUNETWifiProcess, &QProcess::readyReadStandardOutput, this, [this]
            { emit connectSCUNETWifiOutput(m_connectSCUNETWifiProcess->readAllStandardOutput()); });
#endif
}

void PlatformUtils::getWifiInterface()
{
    m_foundInterface.clear();
    m_interfaceSignalEmitted = false; // 重置标志
    m_getWifiInterfaceProcess = new QProcess(this);
    m_getWifiInterfaceProcess->setProcessChannelMode(QProcess::MergedChannels);

    connect(m_getWifiInterfaceProcess, &QProcess::finished, this, &PlatformUtils::onGetWifiInterfaceFinished);
    connect(m_getWifiInterfaceProcess, &QProcess::readyReadStandardOutput, this, &PlatformUtils::processWifiInterfaceOutput);

#if defined(Q_OS_WIN)
    m_wifiInterfaceStep = WifiInterfaceStep::Windows;
    // 对于Windows，使用netsh命令获取WiFi接口
    m_getWifiInterfaceProcess->start("netsh", QStringList() << "wlan"
                                                            << "show"
                                                            << "interfaces");
#elif defined(Q_OS_LINUX)
    // 对于Linux，首先尝试nmcli
    m_wifiInterfaceStep = WifiInterfaceStep::LinuxNMCLI;
    m_getWifiInterfaceProcess->start("nmcli", QStringList() << "-t"
                                                            << "device"
                                                            << "status");
#elif defined(Q_OS_MAC)
    // 对于macOS，首先尝试listallhardwareports
    m_wifiInterfaceStep = WifiInterfaceStep::MacHardwarePorts;
    m_getWifiInterfaceProcess->start("networksetup", QStringList() << "-listallhardwareports");
#endif
}

void PlatformUtils::onOpenHotspotsFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (m_openHotspotsProcess)
    {
        m_openHotspotsProcess->deleteLater();
        m_openHotspotsProcess = nullptr;
    }
    emit openHotspotsFinished(exitCode, exitStatus);
}

void PlatformUtils::onConnectSCUNETWifiFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (m_connectSCUNETWifiProcess)
    {
        m_connectSCUNETWifiProcess->deleteLater();
        m_connectSCUNETWifiProcess = nullptr;
    }
    emit connectSCUNETWifiFinished(exitCode, exitStatus);
}

void PlatformUtils::onGetWifiInterfaceFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    // 处理最后的输出
    processWifiInterfaceOutput();

    // 如果未找到接口，尝试下一种方法
    if (m_foundInterface.isEmpty())
    {
#if defined(Q_OS_LINUX)
        if (m_wifiInterfaceStep == WifiInterfaceStep::LinuxNMCLI)
        {
            // 尝试iwconfig
            m_wifiInterfaceStep = WifiInterfaceStep::LinuxIWConfig;
            m_getWifiInterfaceProcess->start("iwconfig");
            return;
        }
        else if (m_wifiInterfaceStep == WifiInterfaceStep::LinuxIWConfig)
        {
            // 尝试ip命令
            m_wifiInterfaceStep = WifiInterfaceStep::LinuxIP;
            m_getWifiInterfaceProcess->start("ip", QStringList() << "link"
                                                                 << "show");
            return;
        }
#elif defined(Q_OS_MAC)
        if (m_wifiInterfaceStep == WifiInterfaceStep::MacHardwarePorts)
        {
            // 尝试另一种方法
            m_wifiInterfaceStep = WifiInterfaceStep::MacNetworkServices;
            m_getWifiInterfaceProcess->start("networksetup", QStringList() << "-listnetworkserviceorder");
            return;
        }
#endif
    }

    // 清理进程
    if (m_getWifiInterfaceProcess)
    {
        m_getWifiInterfaceProcess->deleteLater();
        m_getWifiInterfaceProcess = nullptr;
    }

    // 只有当信号尚未发出时才发出
    if (!m_interfaceSignalEmitted)
    {
        // 设置默认值
        if (m_foundInterface.isEmpty())
        {
#if defined(Q_OS_MAC)
            m_foundInterface = "en0";
#endif
        }
        m_interfaceSignalEmitted = true;
        emit wifiInterfaceFound(m_foundInterface);
    }
}

void PlatformUtils::processWifiInterfaceOutput()
{
    // 提前处理输出，可以在命令完成前开始分析
    if (!m_getWifiInterfaceProcess)
        return;

    QString output = m_getWifiInterfaceProcess->readAllStandardOutput();

    // 根据当前步骤处理输出
    switch (m_wifiInterfaceStep)
    {
#if defined(Q_OS_WIN)
    case WifiInterfaceStep::Windows:
    {
        static QRegularExpression regexEn("Name\\s+:\\s+(.+)\\r?\\n");
        static QRegularExpression regexCh("名称\\s+:\\s+(.+)\\r?\\n");

        QRegularExpressionMatch matchEn = regexEn.match(output);
        if (matchEn.hasMatch())
        {
            m_foundInterface = matchEn.captured(1).trimmed();
        }
        else
        {
            QRegularExpressionMatch matchCh = regexCh.match(output);
            if (matchCh.hasMatch())
            {
                m_foundInterface = matchCh.captured(1).trimmed();
            }
        }
    }
    break;
#elif defined(Q_OS_LINUX)
    case WifiInterfaceStep::LinuxNMCLI:
    {
        QStringList lines = output.split('\n', Qt::SkipEmptyParts);
        auto it = std::find_if(lines.begin(), lines.end(), [](const QString &line)
                               { return line.contains("wifi"); });
        if (it != lines.end())
        {
            QStringList parts = it->split(':', Qt::SkipEmptyParts);
            if (!parts.isEmpty())
            {
                m_foundInterface = parts.first();
            }
        }
    }
    break;
    case WifiInterfaceStep::LinuxIWConfig:
    {
        QStringList lines = output.split('\n', Qt::SkipEmptyParts);
        auto it = std::find_if(lines.begin(), lines.end(), [](const QString &line)
                               { return line.contains("IEEE 802.11") || line.contains("ESSID:"); });
        if (it != lines.end())
        {
            m_foundInterface = it->split(' ').first();
        }
    }
    break;
    case WifiInterfaceStep::LinuxIP:
    {
        static QRegularExpression regex("\\d+:\\s+(\\w+).*");
        QRegularExpressionMatchIterator i = regex.globalMatch(output);
        while (i.hasNext() && m_foundInterface.isEmpty())
        {
            QRegularExpressionMatch match = i.next();
            QString iface = match.captured(1).trimmed();
            if (iface.startsWith("wl"))
            { // 大多数Linux系统上WiFi接口以wl开头
                m_foundInterface = iface;
                break;
            }
        }
    }
    break;
#elif defined(Q_OS_MAC)
    case WifiInterfaceStep::MacHardwarePorts:
    {
        QStringList lines = output.split('\n', Qt::SkipEmptyParts);
        for (int i = 0; i < lines.size() - 1; i++)
        {
            if (lines[i].contains("Wi-Fi") || lines[i].contains("AirPort"))
            {
                if (i + 1 < lines.size() && lines[i + 1].startsWith("Device:"))
                {
                    m_foundInterface = lines[i + 1].section(':', 1).trimmed();
                    break;
                }
            }
        }
    }
    break;
    case WifiInterfaceStep::MacNetworkServices:
    {
        static QRegularExpression regex("\\((\\d+)\\)\\s+Wi-Fi");
        QRegularExpressionMatch match = regex.match(output);
        if (match.hasMatch())
        {
            // 找到了Wi-Fi服务，默认接口可能是en0
            m_foundInterface = "en0";
        }
    }
    break;
#endif
    default:
        break;
    }

    // 如果已经找到接口，可以提前完成
    if (!m_foundInterface.isEmpty() && !m_interfaceSignalEmitted)
    {
        m_interfaceSignalEmitted = true; // 设置标志，防止重复发送
        m_getWifiInterfaceProcess->kill();
        emit wifiInterfaceFound(m_foundInterface);
    }
}
