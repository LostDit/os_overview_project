#include "servicemanager.h"
#include <QProcess>
#include <QJsonObject>

// Конструктор/деструктор без изменений
ServiceManager::ServiceManager(QObject* parent) : QObject(parent) { }
ServiceManager::~ServiceManager() { }

// Возвращает массив служб (каждый объект содержит: name, load, active, sub, description)
QJsonArray ServiceManager::getServices() const {
    QJsonArray services;
    QProcess process;
    process.start("systemctl", { "list-units", "--type=service", "--all", "--no-pager", "--no-legend" });
    process.waitForFinished();

    QString output = process.readAllStandardOutput();
    QStringList lines = output.split('\n', Qt::SkipEmptyParts);

    for (const QString& line : lines) {
        QStringList parts = line.split(' ', Qt::SkipEmptyParts);
        if (parts.size() < 5) continue;

        QJsonObject service;
        service["name"] = parts[0];
        service["load"] = parts[1];
        service["active"] = parts[2];
        service["sub"] = parts[3];
        service["description"] = parts.mid(4).join(' ');

        services.append(service);
    }
    return services;
}

bool ServiceManager::manageService(const QString& service, const QString& action) {
    QProcess process;
    if      (action == "start")   process.start("systemctl", { "start", service });
    else if (action == "stop")    process.start("systemctl", { "stop", service });
    else if (action == "restart") process.start("systemctl", { "restart", service });
    else                          return false;

    process.waitForFinished();
    return process.exitCode() == 0;
}
