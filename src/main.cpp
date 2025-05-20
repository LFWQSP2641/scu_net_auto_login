#include "Global.h"
#include "SCUNetLoginApplication.h"
#include "Settings.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QFile>
#include <QMutex>
#include <QThread>
#include <QTimer>
#include <QFileInfo>

namespace
{
QFile logFile("application.log");
QMutex logMutex;

// 日志文件最大大小（字节），默认 10MB
constexpr qint64 MAX_LOG_SIZE = 10 * 1024 * 1024;  
// 日志文件最大保留天数，默认 30 天
constexpr int MAX_LOG_DAYS = 30;  

// 检查并处理日志文件
void checkAndRotateLogFile()
{
    QMutexLocker locker(&logMutex); // 加锁保证线程安全
    
    if (!logFile.exists())
        return;
        
    QFileInfo fileInfo(logFile.fileName());
    bool needRotate = false;
    QString reason;
    
    // 检查文件大小
    if (fileInfo.size() > MAX_LOG_SIZE) {
        needRotate = true;
        reason = QString("大小超过 %1 MB").arg(MAX_LOG_SIZE / (1024.0 * 1024.0), 0, 'f', 2);
    }
    
    // 检查文件修改时间
    QDateTime lastModified = fileInfo.lastModified();
    int daysSinceModified = lastModified.daysTo(QDateTime::currentDateTime());
    if (daysSinceModified > MAX_LOG_DAYS) {
        needRotate = true;
        reason = QString("日志文件已超过 %1 天未更新").arg(daysSinceModified);
    }
    
    // 执行清理操作
    if (needRotate) {
        // 先关闭文件
        if (logFile.isOpen()) {
            logFile.close();
        }
        
        // 创建备份文件名
        QString backupName = QString("application_%1.log")
            .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
            
        // 记录一条消息
        qInfo() << "执行日志文件轮转：" << reason;
        
        // 备份旧日志（可选）
        QFile::rename(logFile.fileName(), backupName);
        
        // 重新打开日志文件
        logFile.open(QIODevice::Append | QIODevice::Text);
    }
}

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
    auto s = stderr;
    if (type == QtDebugMsg || type == QtInfoMsg)
        s = stdout;
    fprintf(s, "%s\n", formattedMessage.toLocal8Bit().constData());
    fflush(s);

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
} // namespace

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if (!logFile.open(QIODevice::Append | QIODevice::Text))
    {
        qWarning() << "无法打开日志文件";
    }

    // 在安装消息处理程序之前先检查日志文件
    checkAndRotateLogFile();

    // 设置定时器定期检查日志文件
    QTimer* logRotateTimer = new QTimer(&a);
    // 每天检查一次日志文件
    logRotateTimer->setInterval(24 * 60 * 60 * 1000);
    QObject::connect(logRotateTimer, &QTimer::timeout, checkAndRotateLogFile);
    logRotateTimer->start();

    qInstallMessageHandler(myMessageHandler);

    // 初始化全局设置
    Global::initOnce();
    Settings::initOnce();
    Settings::getSingletonSettings()->saveSettings();

    SCUNetLoginApplication app(&a);
    if (!app.initialize(argc, argv))
    {
        return 1;
    }

    app.startLoginProcess();

    return a.exec();
}
