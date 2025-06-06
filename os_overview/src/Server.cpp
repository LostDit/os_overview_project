#include "server.h"
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDataStream>
#include <QFile>
#include <QDebug>

Server::Server(QObject* parent) : QTcpServer(parent) {
}

Server::~Server() {}

bool Server::start(quint16 port) {
    if (!listen(QHostAddress::Any, port)) {
        qCritical() << "Could not start server:" << errorString();
        return false;
    }
    qInfo() << "Server started on port" << port;
    return true;
}

void Server::startDiscovery(quint16 discoveryPort, quint16 tcpPort) {
    discovery.start(discoveryPort, tcpPort);
}

void Server::incomingConnection(qintptr socketDescriptor) {
    QTcpSocket* client = new QTcpSocket(this);
    if (!client->setSocketDescriptor(socketDescriptor)) {
        qWarning() << "Failed to set socket descriptor";
        client->deleteLater();
        return;
    }

    clientBuffers[client] = QByteArray();
    clientBlockSizes[client] = 0;

    connect(client, &QTcpSocket::readyRead, this, &Server::onClientReadyRead);
    connect(client, &QTcpSocket::disconnected, this, &Server::handleClientDisconnected);

    qInfo() << "New client connected from" << client->peerAddress().toString();
}

void Server::handleClientDisconnected() {
    QTcpSocket* client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return;

    clientBuffers.remove(client);
    clientBlockSizes.remove(client);
    client->deleteLater();
    qInfo() << "Client disconnected";
}

void Server::onClientReadyRead() {
    QTcpSocket* client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return;

    QDataStream in(client);
    in.setVersion(QDataStream::Qt_5_14);
    in.setByteOrder(QDataStream::BigEndian);

    quint32 &blockSize = clientBlockSizes[client];
    QByteArray &buffer = clientBuffers[client];

    if (blockSize == 0) {
        if (client->bytesAvailable() < static_cast<int>(sizeof(quint32))) return;
        in >> blockSize;
    }
    if (client->bytesAvailable() < static_cast<int>(blockSize)) return;

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
    if (!doc.isObject()) return;

    QJsonObject request = doc.object();
    QString method = request["method"].toString();
    int id = request["id"].toInt(-1);

    QJsonObject response;
    if (id >= 0) response["id"] = id;

    if (method == "getUserList") {
        response["result"] = userManager.getUserListAsJsonArray();
    }
    else if (method == "getSystemInfo") {
        response["result"] = systemInfo.collectSystemInfo();
    }
    else if (method == "getFileSystem") {
        response["result"] = fileManager.getFileSystemInfo(request["params"].toObject()["path"].toString());
    }
    else if (method == "getProcessList") {
        response["result"] = processManager.getProcessListAsJsonArray();
    }
    else if (method == "getServiceList") {
        response["result"] = serviceManager.getServices();
    }
    else if (method == "addUser") {
        auto p = request["params"].toObject();
        bool ok = userManager.addUser(p["username"].toString(), p["password"].toString());
        response[ok ? "result" : "error"] = ok ? QJsonObject{{"status", "success"}} : QJsonObject{{"code", -32001}, {"message", "Failed to add user"}};
    }
    else if (method == "removeUser") {
        auto p = request["params"].toObject();
        bool ok = userManager.removeUser(p["username"].toString());
        response[ok ? "result" : "error"] = ok ? QJsonObject{{"status", "success"}} : QJsonObject{{"code", -32002}, {"message", "Failed to remove user"}};
    }
    else if (method == "changeUserPassword") {
        auto p = request["params"].toObject();
        bool ok = userManager.changePassword(p["username"].toString(), p["newPassword"].toString());
        response[ok ? "result" : "error"] = ok ? QJsonObject{{"status", "success"}} : QJsonObject{{"code", -32003}, {"message", "Failed to change password"}};
    }
    else if (method == "setFilePermissions") {
        auto p = request["params"].toObject();
        bool ok = fileManager.setPermissions(p["filePath"].toString(), p["permissions"].toString());
        response[ok ? "result" : "error"] = ok ? QJsonObject{{"status", "success"}} : QJsonObject{{"code", -32004}, {"message", "Failed to set permissions"}};
    }
    else if (method == "manageService") {
        auto p = request["params"].toObject();
        bool ok = serviceManager.manageService(p["serviceName"].toString(), p["action"].toString());
        response[ok ? "result" : "error"] = ok ? QJsonObject{{"status", "success"}} : QJsonObject{{"code", -32005}, {"message", "Failed to manage service"}};
    }
    else if (method == "uploadFile") {
        auto p = request["params"].toObject();
        QFile file(p["remotePath"].toString());
        if (file.open(QIODevice::WriteOnly)) {
            file.write(QByteArray::fromBase64(p["data"].toString().toUtf8()));
            file.close();
            response["result"] = QJsonObject{{"status", "success"}};
        } else {
            response["error"] = QJsonObject{{"code", -32006}, {"message", "Failed to write file"}};
        }
    }
    else if (method == "downloadFile") {
        auto p = request["params"].toObject();
        QFile file(p["remotePath"].toString());
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray data = file.readAll();
            file.close();
            response["result"] = QJsonObject{
                {"savePath", p["savePath"].toString()},
                {"data", QString::fromUtf8(data.toBase64())}
            };
        } else {
            response["error"] = QJsonObject{{"code", -32007}, {"message", "Failed to read file"}};
        }
    }
    else {
        response["error"] = QJsonObject{{"code", -32601}, {"message", "Unknown method"}};
    }

    sendJsonResponse(client, response);
}

void Server::sendJsonResponse(QTcpSocket* client, const QJsonObject& response) {
    QJsonDocument doc(response);
    QByteArray payload = doc.toJson(QJsonDocument::Compact);
    quint32 len = static_cast<quint32>(payload.size());

    QByteArray packet;
    QDataStream stream(&packet, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << len;
    packet.append(payload);

    client->write(packet);
    client->flush();
}
