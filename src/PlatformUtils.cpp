#include "PlatformUtils.h"

#include <QFile>
#include <QProcess>

namespace
{
#ifdef Q_OS_WIN
// Julius Hardt created this script to toggle the Mobile Hotspot on Windows 10/11.
// https://stackoverflow.com/users/7006729/julius-hardt
// https://stackoverflow.com/a/55563418
// https://gist.github.com/LFWQSP2641/0de8d6db8c00e68f562ea5042f707a2c
constexpr char windows10HotspotPowerShellScript[] = R"(
[Windows.System.UserProfile.LockScreen,Windows.System.UserProfile,ContentType=WindowsRuntime] | Out-Null
Add-Type -AssemblyName System.Runtime.WindowsRuntime
$asTaskGeneric = ([System.WindowsRuntimeSystemExtensions].GetMethods() | ? { $_.Name -eq 'AsTask' -and $_.GetParameters().Count -eq 1 -and $_.GetParameters()[0].ParameterType.Name -eq 'IAsyncOperation`1' })[0]
Function Await($WinRtTask, $ResultType) {
    $asTask = $asTaskGeneric.MakeGenericMethod($ResultType)
    $netTask = $asTask.Invoke($null, @($WinRtTask))
    $netTask.Wait(-1) | Out-Null
    $netTask.Result
}
Function AwaitAction($WinRtAction) {
    $asTask = ([System.WindowsRuntimeSystemExtensions].GetMethods() | ? { $_.Name -eq 'AsTask' -and $_.GetParameters().Count -eq 1 -and !$_.IsGenericMethod })[0]
    $netTask = $asTask.Invoke($null, @($WinRtAction))
    $netTask.Wait(-1) | Out-Null
}
$connectionProfile = [Windows.Networking.Connectivity.NetworkInformation,Windows.Networking.Connectivity,ContentType=WindowsRuntime]::GetInternetConnectionProfile()
$tetheringManager = [Windows.Networking.NetworkOperators.NetworkOperatorTetheringManager,Windows.Networking.NetworkOperators,ContentType=WindowsRuntime]::CreateFromConnectionProfile($connectionProfile)
Await ($tetheringManager.StartTetheringAsync()) ([Windows.Networking.NetworkOperators.NetworkOperatorTetheringOperationResult])
)";
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
      m_connectSCUNETWifiProcess {nullptr}
{
}

void PlatformUtils::openHotspots()
{
#if defined(Q_OS_WIN)
    m_openHotspotsProcess = new QProcess(this);
    m_openHotspotsProcess->setProcessChannelMode(QProcess::MergedChannels);
    m_openHotspotsProcess->start("powershell", QStringList() << "-Command" << windows10HotspotPowerShellScript);
    connect(m_openHotspotsProcess, &QProcess::finished, this, &PlatformUtils::onOpenHotspotsFinished);
    connect(m_openHotspotsProcess, &QProcess::readyReadStandardOutput, this, [this]
            { emit openHotspotsOutput(m_openHotspotsProcess->readAllStandardOutput()); });

#elif defined(Q_OS_MAC)
    m_openHotspotsProcess = new QProcess(this);
    m_openHotspotsProcess->setProcessChannelMode(QProcess::MergedChannels);
    m_openHotspotsProcess->start("osascript", QStringList() << "-e" << macHotspotScript);
    connect(m_openHotspotsProcess, &QProcess::finished, this, &PlatformUtils::onOpenHotspotsFinished);
    connect(m_openHotspotsProcess, &QProcess::readyReadStandardOutput, this, [this]
            { emit openHotspotsOutput(m_openHotspotsProcess->readAllStandardOutput()); });
#endif
}

void PlatformUtils::connectSCUNETWifi()
{
    #if defined(Q_OS_WIN)
    m_connectSCUNETWifiProcess = new QProcess(this);
    m_connectSCUNETWifiProcess->setProcessChannelMode(QProcess::MergedChannels);
    m_connectSCUNETWifiProcess->start("netsh", QStringList() << "wlan"
                                                             << "connect"
                                                             << "name=\"SCUNET\"");
    connect(m_connectSCUNETWifiProcess, &QProcess::finished, this, &PlatformUtils::onConnectSCUNETWifiFinished);
    connect(m_connectSCUNETWifiProcess, &QProcess::readyReadStandardOutput, this, [this]
            { emit connectSCUNETWifiOutput(m_connectSCUNETWifiProcess->readAllStandardOutput()); });
#elif defined(Q_OS_LINUX)
    m_connectSCUNETWifiProcess = new QProcess(this);
    m_connectSCUNETWifiProcess->setProcessChannelMode(QProcess::MergedChannels);
    m_connectSCUNETWifiProcess->start("nmcli", QStringList() << "device" 
                                                             << "wifi" 
                                                             << "connect" 
                                                             << "SCUNET");
    connect(m_connectSCUNETWifiProcess, &QProcess::finished, this, &PlatformUtils::onConnectSCUNETWifiFinished);
    connect(m_connectSCUNETWifiProcess, &QProcess::readyReadStandardOutput, this, [this]
            { emit connectSCUNETWifiOutput(m_connectSCUNETWifiProcess->readAllStandardOutput()); });
#elif defined(Q_OS_MAC)
    m_connectSCUNETWifiProcess = new QProcess(this);
    m_connectSCUNETWifiProcess->setProcessChannelMode(QProcess::MergedChannels);
    m_connectSCUNETWifiProcess->start("networksetup", QStringList() << "-setairportnetwork" 
                                                                    << "en0" 
                                                                    << "SCUNET");
    connect(m_connectSCUNETWifiProcess, &QProcess::finished, this, &PlatformUtils::onConnectSCUNETWifiFinished);
    connect(m_connectSCUNETWifiProcess, &QProcess::readyReadStandardOutput, this, [this]
            { emit connectSCUNETWifiOutput(m_connectSCUNETWifiProcess->readAllStandardOutput()); });
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
