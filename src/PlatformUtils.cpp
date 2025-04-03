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
      m_connectSCUNETWifiProcess {nullptr}
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
    m_connectSCUNETWifiProcess = new QProcess(this);
    m_connectSCUNETWifiProcess->setProcessChannelMode(QProcess::MergedChannels);
#if defined(Q_OS_WIN)
    m_connectSCUNETWifiProcess->start("netsh", QStringList() << "wlan"
                                                             << "connect"
                                                             << "name=\"SCUNET\"");
#elif defined(Q_OS_LINUX)
    m_connectSCUNETWifiProcess->start("nmcli", QStringList() << "device"
                                                             << "wifi"
                                                             << "connect"
                                                             << "SCUNET");
#elif defined(Q_OS_MAC)
    m_connectSCUNETWifiProcess->start("networksetup", QStringList() << "-setairportnetwork"
                                                                    << "en0"
                                                                    << "SCUNET");
#endif
    connect(m_connectSCUNETWifiProcess, &QProcess::finished, this, &PlatformUtils::onConnectSCUNETWifiFinished);
    connect(m_connectSCUNETWifiProcess, &QProcess::readyReadStandardOutput, this, [this]
            { emit connectSCUNETWifiOutput(m_connectSCUNETWifiProcess->readAllStandardOutput()); });
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
