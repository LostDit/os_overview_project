#ifndef SYSTEMINFO_H
#define SYSTEMINFO_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>

class SystemInfo : public QObject
{
    Q_OBJECT
public:
    explicit SystemInfo(QObject *parent = nullptr);
    ~SystemInfo();

    QJsonObject collectSystemInfo() const;

private:
    QString getOSInfo() const;
    QString getCpuInfo() const;
    int getCpuCores() const;
    QJsonObject getCpuLoad() const;
    QJsonArray getCpuLoadPerCore() const;
    double getCpuTemperature() const;
    double getHddTemperature() const;
    QJsonObject getMemoryInfo() const;
    QJsonArray getDiskInfo() const;
    QString getUptime() const;
    QJsonObject getTemperatureInfo() const;
    QJsonArray getPeripheralDevices() const;
    QJsonObject getNetworkInfo() const;
};

#endif // SYSTEMINFO_H
