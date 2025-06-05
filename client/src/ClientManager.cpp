#include "ClientManager.h"
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDataStream>
#include <QDebug>

ClientManager::ClientManager(QObject* parent)
    : QObject(parent), blockSize(0), nextId(1)
{
    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::connected, this, &ClientManager::onConnected);
    connect(socket, &QTcpSocket::readyRead, this, &ClientManager::onReadyRead);
    connect(socket,
            QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error),
            this, &ClientManager::onErrorOccurred);
}

void ClientManager::uploadFile(const QString& localPath, const QString& remotePath) {
    QFile file(localPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open file for upload:" << localPath;
        emit fileUploadFinished(false, "Failed to open file");
        return;
    }
    QByteArray fileData = file.readAll();
    file.close();

    QJsonObject request;
    request["method"] = "uploadFile";
    QJsonObject params;
    params["remotePath"] = remotePath;
    params["data"] = QString::fromUtf8(fileData.toBase64());
    request["params"] = params;
    sendJson(request, "uploadFile");
}

void ClientManager::downloadFile(const QString& remotePath, const QString& localPath) {
    QJsonObject request;
    request["method"] = "downloadFile";
    QJsonObject params;
    params["remotePath"] = remotePath;
    params["savePath"] = localPath;
    request["params"] = params;
    sendJson(request, "downloadFile");
    // Здесь сервер должен отправить ответ с base64, который клиент расшифрует в onReadyRead
    // и сохранит в файл, затем вызовет fileDownloadFinished
}

void ClientManager::connectToServer(const QString& host, quint16 port) {
    socket->connectToHost(host, port);
}

void ClientManager::onConnected() {
    emit connected();
}

void ClientManager::onErrorOccurred(QAbstractSocket::SocketError socketError) {
    Q_UNUSED(socketError);
    emit connectionError(socket->errorString());
}

void ClientManager::sendJson(const QJsonObject& baseObj, const QString& methodName) {
    if (socket->state() != QAbstractSocket::ConnectedState) {
        qWarning() << "Trying to send data while not connected";
        return;
    }
    // 1) делаем копию JSON
    QJsonObject obj = baseObj;
    int id = nextId++;
    obj["id"] = id;
    pendingRequests[id] = methodName;

    QJsonDocument doc(obj);
    QByteArray data = doc.toJson(QJsonDocument::Compact);

    QByteArray packet;
    quint32 len = data.size();
    QDataStream stream(&packet, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << len;
    packet.append(data);
    socket->write(packet);
}

void ClientManager::requestUserList() {
    QJsonObject request;
    request["method"] = "getUserList";
    request["params"] = QJsonObject();
    sendJson(request, "getUserList");
}

void ClientManager::requestSystemInfo() {
    QJsonObject request;
    request["method"] = "getSystemInfo";
    sendJson(request, "getSystemInfo");
}

void ClientManager::requestFileSystem(const QString& path) {
    QJsonObject request;
    request["method"] = "getFileSystem";
    request["params"] = QJsonObject{{"path", path}};
    sendJson(request, "getFileSystem");
}

void ClientManager::requestProcessList() {
    QJsonObject request;
    request["method"] = "getProcessList";
    sendJson(request, "getProcessList");
}

// И т. д. для остальных методов: addUser, removeUser, changeUserPassword, setFilePermissions, manageService

void ClientManager::onReadyRead() {
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_5_14);
    in.setByteOrder(QDataStream::BigEndian);

    // аналогично: ждём 4 байта, чтобы прочитать blockSize
    if (blockSize == 0) {
        if (socket->bytesAvailable() < static_cast<int>(sizeof(quint32)))
            return;
        in >> blockSize;
    }
    if (socket->bytesAvailable() < blockSize)
        return;

    QByteArray data;
    data.resize(blockSize);
    in.readRawData(data.data(), blockSize);
    blockSize = 0;

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error:" << parseError.errorString();
        return;
    }

    QJsonObject response = doc.object();
    // Ищем id ответа
    if (!response.contains("id")) {
        qWarning() << "JSON-RPC response without id";
        return;
    }
    int id = response["id"].toInt(-1);
    if (!pendingRequests.contains(id)) {
        qWarning() << "Unknown id in response:" << id;
        return;
    }
    QString method = pendingRequests.take(id); // достаём и стираем
    // Теперь смотрим, есть ли "error"
    if (response.contains("error")) {
        QJsonObject err = response["error"].toObject();
        qWarning() << "Server returned error for" << method << ":" << err["message"].toString();
        return;
    }
    // Идём в "result"
    if (!response.contains("result")) {
        qWarning() << "Response for" << method << "has no result";
        return;
    }

    // Смотрим, какой это метод:
    if (method == "getUserList") {
        QJsonArray array = response["result"].toArray();
        QStringList list;
        for (const auto& val : array) {
            list << val.toString();
        }
        emit userListReceived(list);
    }
    else if (method == "getSystemInfo") {
        emit systemInfoReceived(response["result"].toObject());
    }
    else if (method == "getFileSystem") {
        emit fileSystemReceived(response["result"].toArray());
    }
    else if (method == "getProcessList") {
        emit processListReceived(response["result"].toArray());
    }
    else {
        // Любой другой метод (addUser, removeUser, setFilePermissions, manageService)
        // Например, просто эмитим, что этот метод отработал и бросаем результат (или пустой):
        emit operationFinished(method, response["result"].toObject());
    }
}
