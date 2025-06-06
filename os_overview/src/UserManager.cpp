#include "usermanager.h"
#include <QProcess>

UserManager::UserManager(QObject* parent) : QObject(parent) {}
UserManager::~UserManager() {}

QJsonArray UserManager::getUserListAsJsonArray() const {
    QJsonArray array;
    QProcess process;
    process.start("cut", { "-d:", "-f1", "/etc/passwd" });
    process.waitForFinished();
    QString output = process.readAllStandardOutput();
    QStringList users = output.split('\n', Qt::SkipEmptyParts);
    for (const QString& u : users) {
        array.append(u);
    }
    return array;
}

bool UserManager::addUser(const QString& username, const QString& password) {
    QProcess process;
    process.start("useradd", { username });
    process.waitForFinished();
    if (process.exitCode() != 0) return false;

    process.start("chpasswd");
    process.write(QString("%1:%2").arg(username, password).toUtf8());
    process.closeWriteChannel();
    process.waitForFinished();
    return process.exitCode() == 0;
}

bool UserManager::removeUser(const QString& username) {
    QProcess process;
    process.start("userdel", { "-r", username });
    process.waitForFinished();
    return process.exitCode() == 0;
}

bool UserManager::changePassword(const QString& username, const QString& password) {
    QProcess process;
    process.start("chpasswd");
    process.write(QString("%1:%2").arg(username, password).toUtf8());
    process.closeWriteChannel();
    process.waitForFinished();
    return process.exitCode() == 0;
}
