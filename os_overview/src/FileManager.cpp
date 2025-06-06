#include "filemanager.h"
#include <QDir>
#include <QFileInfo>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QProcess>

// Конструктор/деструктор остаются без изменений
FileManager::FileManager(QObject *parent) : QObject(parent) { }
FileManager::~FileManager() { }

// Возвращает QJsonArray, где каждое entry — результат fileInfoToJson()
QJsonArray FileManager::getFileSystemInfo(const QString &path) const {
    QJsonArray files;
    QDir dir(path.isEmpty() ? QDir::rootPath() : path);

    if (!dir.exists()) return files;
    const auto fileList = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden);
    for (const QFileInfo &info : fileList) {
        files.append(fileInfoToJson(info));
    }
    return files;
}

// Преобразуем права доступа в строку «rwxrwxrwx»
QString permissionsToString(QFile::Permissions permissions) {
    QString result;
    result.reserve(9);

    result.append((permissions & QFile::ReadOwner) ? 'r' : '-');
    result.append((permissions & QFile::WriteOwner) ? 'w' : '-');
    result.append((permissions & QFile::ExeOwner) ? 'x' : '-');
    result.append((permissions & QFile::ReadGroup) ? 'r' : '-');
    result.append((permissions & QFile::WriteGroup) ? 'w' : '-');
    result.append((permissions & QFile::ExeGroup) ? 'x' : '-');
    result.append((permissions & QFile::ReadOther) ? 'r' : '-');
    result.append((permissions & QFile::WriteOther) ? 'w' : '-');
    result.append((permissions & QFile::ExeOther) ? 'x' : '-');

    return result;
}

QJsonObject FileManager::fileInfoToJson(const QFileInfo &info) const {
    QJsonObject file;
    file["name"] = info.fileName();
    file["path"] = info.absoluteFilePath();
    file["is_dir"] = info.isDir();             // вместо «type» теперь boolean
    file["size"] = static_cast<qint64>(info.size());
    file["permissions"] = permissionsToString(info.permissions());
    file["owner"] = info.owner();
    file["group"] = info.group();
    file["created"] = info.birthTime().toString(Qt::ISODate);
    file["modified"] = info.lastModified().toString(Qt::ISODate);
    return file;
}

bool FileManager::setPermissions(const QString &path, const QString &perms) {
    // Заменить на поддержку ACL (setfacl)
    QProcess process;
    process.start("setfacl", {"-m", perms, path});
    process.waitForFinished();
    return (process.exitCode() == 0);
}
