#include "processmanager.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QProcess>
#include <QRegularExpression>
#include <QTextStream>

QJsonArray ProcessManager::getProcessListAsJsonArray() const {
    QJsonArray processesArray;

#ifdef Q_OS_LINUX
    QProcess process;
    process.start("ps", QStringList() << "-eo" << "pid,comm,state,ppid,utime,stime,rss,user,args");
    process.waitForFinished();
    QByteArray output = process.readAllStandardOutput();
    QStringList lines = QString(output).split('\n', Qt::SkipEmptyParts);
    for (const QString& line : lines) {
        QStringList parts = line.trimmed().split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        if (parts.size() >= 9) {
            QJsonObject processObj;
            processObj["pid"] = parts[0].toInt();
            processObj["name"] = parts[1];
            processObj["state"] = parts[2];
            processObj["ppid"] = parts[3].toInt();
            processObj["utime"] = parts[4].toLongLong();
            processObj["stime"] = parts[5].toLongLong();
            processObj["rss"] = parts[6].toLongLong() * 1024; // rss в KB, конвертируем в байты
            processObj["user"] = parts[7];
            processObj["cmdline"] = parts.mid(8).join(" ");
            processesArray.append(processObj);
        }
    }
#elif defined(Q_OS_WIN)
    QProcess process;
    process.start("tasklist", QStringList() << "/v" << "/fo" << "csv");
    process.waitForFinished();
    QByteArray output = process.readAllStandardOutput();
    QStringList lines = QString(output).split('\n', Qt::SkipEmptyParts);
    if (lines.size() > 1) {
        for (int i = 1; i < lines.size(); ++i) {
            QString line = lines[i];
            line = line.replace("\"", "");
            QStringList parts = line.split(",", Qt::KeepEmptyParts);
            if (parts.size() >= 9) {
                QJsonObject processObj;
                processObj["name"] = parts[0];
                processObj["pid"] = parts[1].toInt();
                QString memUsage = parts[4].replace(" K", "").replace(",", "");
                processObj["rss"] = memUsage.toLongLong() * 1024; // конвертируем в байты
                processObj["state"] = parts[5]; // статус, например, "Running"
                processObj["user"] = parts[6];
                processesArray.append(processObj);
            }
        }
    }
#else
    #error "Unsupported platform"
#endif

    return processesArray;
}
