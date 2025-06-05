#ifndef SERVICEMANAGER_H
#define SERVICEMANAGER_H

#include <QObject>
#include <QJsonArray>

class ServiceManager : public QObject
{
    Q_OBJECT
public:
    explicit ServiceManager(QObject *parent = nullptr);
    ~ServiceManager();

    QJsonArray getServices() const;
    bool manageService(const QString &service, const QString &action);
};

#endif // SERVICEMANAGER_H
