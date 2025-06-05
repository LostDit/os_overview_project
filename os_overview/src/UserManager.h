#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <QObject>
#include <QJsonArray>

class UserManager : public QObject
{
    Q_OBJECT
public:
    explicit UserManager(QObject *parent = nullptr);
    ~UserManager();

    QJsonArray getUserListAsJsonArray() const;
    bool addUser(const QString &username, const QString &password);
    bool removeUser(const QString &username);
    bool changePassword(const QString &username, const QString &password);
};

#endif // USERMANAGER_H
