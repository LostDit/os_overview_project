#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QObject>
#include <QJsonArray>
#include <QJsonObject>
#include <QFileInfo>

class FileManager : public QObject
{
    Q_OBJECT
public:
    explicit FileManager(QObject *parent = nullptr);
    ~FileManager();

    QJsonArray getFileSystemInfo(const QString &path) const;
    bool setPermissions(const QString &path, const QString &perms);

private:
    QJsonObject fileInfoToJson(const QFileInfo &info) const;
};

#endif // FILEMANAGER_H
