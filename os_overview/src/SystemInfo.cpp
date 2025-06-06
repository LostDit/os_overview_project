#include "systeminfo.h"
#include <QFile>
#include <QTextStream>
#include <QJsonArray>
#include <QJsonObject>
#include <QProcess>
#include <QDateTime>
#include <QDir>

SystemInfo::SystemInfo(QObject* parent) : QObject(parent) { }
SystemInfo::~SystemInfo() { }

QJsonObject SystemInfo::collectSystemInfo() const {
    QJsonObject info;

    info["os_name"]       = getOSInfo();
    info["cpu_model"]     = getCpuInfo();
    info["cpu_cores"]     = getCpuCores();
    info["cpu_load"]      = getCpuLoad();
    info["cpu_load_per_core"] = getCpuLoadPerCore();
    info["memory"]        = getMemoryInfo();
    info["disks"]         = getDiskInfo();
    info["temperature"]   = getTemperatureInfo();
    info["uptime"]        = getUptime();
    info["peripherals"]   = getPeripheralDevices();
    info["timestamp"]     = QDateTime::currentDateTime().toString(Qt::ISODate);

    return info;
}

QString SystemInfo::getOSInfo() const {
    QFile file("/etc/os-release");
    if (!file.open(QIODevice::ReadOnly)) return "Unknown";
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.startsWith("PRETTY_NAME=")) {
            return line.mid(13).replace("\"", "");
        }
    }
    return "Unknown";
}

QString SystemInfo::getCpuInfo() const {
    QFile file("/proc/cpuinfo");
    if (!file.open(QIODevice::ReadOnly)) return "Unknown";
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.startsWith("model name")) {
            return line.split(':').value(1).trimmed();
        }
    }
    return "Unknown";
}

int SystemInfo::getCpuCores() const {
    QFile file("/proc/cpuinfo");
    if (!file.open(QIODevice::ReadOnly)) return 0;
    QTextStream in(&file);
    int count = 0;
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.startsWith("processor")) {
            count++;
        }
    }
    return count;
}

QJsonObject SystemInfo::getCpuLoad() const {
    QJsonObject cpuLoad;
    QFile file("/proc/stat");
    if (!file.open(QIODevice::ReadOnly)) return cpuLoad;
    QTextStream in(&file);
    QString line = in.readLine(); // перва€ строка Ч общий load
    QStringList values = line.split(' ', Qt::SkipEmptyParts);
    if (values.size() < 5) return cpuLoad;

    qint64 user   = values[1].toLongLong();
    qint64 nice   = values[2].toLongLong();
    qint64 system = values[3].toLongLong();
    qint64 idle   = values[4].toLongLong();
    qint64 total  = user + nice + system + idle;

    double usagePercent = (total > 0) ? (100.0 * (user + nice + system) / total) : 0.0;
    cpuLoad["usage"]       = usagePercent;
    cpuLoad["temperature"] = getCpuTemperature();

    return cpuLoad;
}

QJsonArray SystemInfo::getCpuLoadPerCore() const {
    QJsonArray loads;
    QFile file("/proc/stat");
    if (!file.open(QIODevice::ReadOnly)) return loads;
    QTextStream in(&file);
    double maxFreq = 0.0; // ћакс. частота (примерно)
    QFile cpuFreq("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq");
    if (cpuFreq.open(QIODevice::ReadOnly)) {
        maxFreq = cpuFreq.readAll().trimmed().toDouble() / 1000000.0; // в GHz
    } else {
        maxFreq = 5.0; // «начение по умолчанию, если не удалось получить
    }

    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.startsWith("cpu") && line[3].isDigit()) { // cpu0, cpu1, etc.
            QStringList values = line.split(' ', Qt::SkipEmptyParts);
            if (values.size() < 5) continue;
            qint64 user = values[1].toLongLong();
            qint64 nice = values[2].toLongLong();
            qint64 system = values[3].toLongLong();
            qint64 idle = values[4].toLongLong();
            qint64 total = user + nice + system + idle;
            double usagePercent = (total > 0) ? (100.0 * (user + nice + system) / total) : 0.0;
            double currentFreq = (usagePercent / 100.0) * maxFreq; // ѕримерное вычисление текущей частоты
            loads.append(QString("%1GHz/%2GHz").arg(currentFreq, 0, 'f', 1).arg(maxFreq, 0, 'f', 1));
        }
    }
    return loads;
}

double SystemInfo::getCpuTemperature() const {
    QFile file("/sys/class/thermal/thermal_zone0/temp");
    if (!file.open(QIODevice::ReadOnly)) return 0.0;
    double temp = file.readAll().trimmed().toDouble() / 1000.0;
    return temp;
}

QJsonObject SystemInfo::getMemoryInfo() const {
    QJsonObject memory;
    QFile file("/proc/meminfo");
    if (!file.open(QIODevice::ReadOnly)) return memory;

    qint64 total = 0, free = 0, available = 0;
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.startsWith("MemTotal:")) {
            total = line.split(' ', Qt::SkipEmptyParts)[1].toLongLong();
        }
        else if (line.startsWith("MemFree:")) {
            free = line.split(' ', Qt::SkipEmptyParts)[1].toLongLong();
        }
        else if (line.startsWith("MemAvailable:")) {
            available = line.split(' ', Qt::SkipEmptyParts)[1].toLongLong();
        }
    }

    memory["total_mb"]     = total / 1024;
    memory["used_mb"]      = (total - free) / 1024;
    memory["available_mb"] = available / 1024;
    memory["usage"]        = QString("%1MB/%2MB").arg((total - free) / 1024).arg(total / 1024);
    memory["usage_percent"]= (total > 0) ? (100.0 * (total - free) / total) : 0.0;
    return memory;
}

QJsonArray SystemInfo::getDiskInfo() const {
    QJsonArray disks;
    QProcess process;
    process.start("df", { "-h", "--output=target,size,used,avail,pcent" });
    process.waitForFinished();

    QString output = process.readAllStandardOutput();
    QStringList lines = output.split('\n', Qt::SkipEmptyParts);

    for (int i = 1; i < lines.size(); ++i) {
        QStringList parts = lines[i].split(' ', Qt::SkipEmptyParts);
        if (parts.size() < 5) continue;
        QJsonObject disk;
        disk["mount_point"]  = parts[0];
        disk["total_size"]   = parts[1];
        disk["used"]         = parts[2];
        disk["available"]    = parts[3];
        disk["usage_percent"]= parts[4];
        disks.append(disk);
    }
    return disks;
}

QString SystemInfo::getUptime() const {
    QFile file("/proc/uptime");
    if (!file.open(QIODevice::ReadOnly)) return "Unknown";
    QTextStream in(&file);
    double upSeconds = in.readLine().split(' ').value(0).toDouble();
    int days = static_cast<int>(upSeconds) / 86400;
    int hours = (static_cast<int>(upSeconds) % 86400) / 3600;
    int mins  = (static_cast<int>(upSeconds) % 3600) / 60;
    return QString("%1d %2h %3m").arg(days).arg(hours).arg(mins);
}

QJsonObject SystemInfo::getTemperatureInfo() const {
    QJsonObject temps;
    double cpuTemp = getCpuTemperature();
    if (cpuTemp > 0.0) temps["cpu"] = cpuTemp;
    else               temps["cpu"] = QString("N/A");

    double hddTemp = getHddTemperature();
    if (hddTemp > 0.0) temps["hdd"] = hddTemp;
    else               temps["hdd"] = QString("N/A");

    return temps;
}

double SystemInfo::getHddTemperature() const {
    QDir dir("/sys/class/scsi_disk/");
    QStringList devices = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString& device : devices) {
        QFile file(QString("/sys/class/scsi_disk/%1/device/enclosure/device:0:0/temperature").arg(device));
        if (file.open(QIODevice::ReadOnly)) {
            QString tempStr = file.readAll().trimmed();
            bool ok;
            double temp = tempStr.toDouble(&ok);
            if (ok) return temp;
        }
    }
    return 0.0; // N/A в случае ошибки
}

QJsonArray SystemInfo::getPeripheralDevices() const {
    QJsonArray devices;
    QProcess process;
    process.start("lsusb");
    process.waitForFinished();
    QString output = process.readAllStandardOutput();
    QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    for (const QString& line : lines) {
        QJsonObject device;
        device["description"] = line;
        devices.append(device);
    }
    return devices;
}
