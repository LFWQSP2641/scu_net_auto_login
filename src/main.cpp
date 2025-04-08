#include "Global.h"
#include "SCUNetLoginApplication.h"
#include "Settings.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QFile>
#include <QMutex>
#include <QThread>

namespace
{
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

    qInstallMessageHandler(myMessageHandler);

    // 初始化全局设置
    Global::initOnce();
    Settings::initOnce();

    SCUNetLoginApplication app(&a);
    if (!app.initialize(argc, argv))
    {
        return 1;
    }

    app.startLoginProcess();

    return a.exec();
}
