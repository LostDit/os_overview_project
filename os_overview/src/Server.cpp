#include "server.h"
#include "usermanager.h"
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>
#include <QDataStream>
#include <QDebug>

// --------------------------------------------------------------------------------
// TODO: ��������������, ��� � server.h ��� �������� ����� Server : public QTcpServer
// � ���� ����:
//    NetworkDiscovery discovery;
//    FileManager fileManager;
//    UserManager userManager;
//    ServiceManager serviceManager;
//    SystemInfo systemInfo;
//    QMap<QTcpSocket*, QByteArray> clientBuffers;
//    QMap<QTcpSocket*, quint32>    clientBlockSizes;
// --------------------------------------------------------------------------------

Server::Server(QObject* parent) : QTcpServer(parent) {}

void Server::startDiscovery(quint16 discoveryPort, quint16 tcpPort) {
    discovery.start(discoveryPort, tcpPort);
}

Server::~Server() {
    // ��� ������������ ������� ��������� ������������� �� �������� QObject.
}

bool Server::start(quint16 port) {
    if (!listen(QHostAddress::Any, port)) {
        qCritical() << "Could not start server:" << errorString();
        return false;
    }
    qInfo() << "Server started on port" << port;
    return true;
}

void Server::incomingConnection(qintptr socketDescriptor) {
    QTcpSocket* client = new QTcpSocket(this);
    if (!client->setSocketDescriptor(socketDescriptor)) {
        qWarning() << "Failed to set socket descriptor";
        client->deleteLater();
        return;
    }

    // �������������� ����� � blockSize ��� ����� �������
    clientBuffers[client] = QByteArray();
    clientBlockSizes[client] = 0;

    connect(client, &QTcpSocket::readyRead, this, &Server::onClientReadyRead);
    connect(client, &QTcpSocket::disconnected, this, &Server::handleClientDisconnected);

    qInfo() << "New client connected from" << client->peerAddress().toString();
}

// ���� ��� ���������� �������: ������� ��� ��������� ������
void Server::handleClientDisconnected() {
    QTcpSocket* client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return;

    clientBuffers.remove(client);
    clientBlockSizes.remove(client);
    client->deleteLater();
    qInfo() << "Client disconnected";
}

// ���� ��� ������ ������ �� ������� (�������� � ���������� �������)
void Server::onClientReadyRead() {
    QTcpSocket* client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return;

    QDataStream in(client);
    in.setVersion(QDataStream::Qt_5_14);
    in.setByteOrder(QDataStream::BigEndian);

    quint32 &blockSize = clientBlockSizes[client];
    QByteArray &buffer = clientBuffers[client];

    // 1) ���� ��� �� ����� ������ ���������� JSON-������, ������ 4-�������� ���������
    if (blockSize == 0) {
        if (client->bytesAvailable() < static_cast<int>(sizeof(quint32)))
            return; // ���� ���� ������ ���� �� 4 �����
        in >> blockSize;
    }
    // 2) ����, ���� ��������� ���� payload
    if (client->bytesAvailable() < static_cast<int>(blockSize))
        return; // ��� �� ���� �payload�

    // 3) ������ ����� blockSize ����
    QByteArray data;
    data.resize(blockSize);
    in.readRawData(data.data(), blockSize);
    blockSize = 0; // ���������� ��� ���������� ������

    // 4) ������ JSON
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error:" << parseError.errorString();
        return;
    }
    if (!doc.isObject()) {
        qWarning() << "Invalid JSON-RPC: not an object";
        return;
    }

    QJsonObject request = doc.object();

    // 5) ��������� �method� � �id�
    QString method = request.value("method").toString();
    int id = request.value("id").toInt(-1);
    QJsonObject response;
    if (id >= 0) {
        response["id"] = id;
    }

    // 6) ������������ �����
    if (method == "getUserList") {
        QJsonArray arr = userManager.getUserListAsJsonArray();
        response["result"] = arr;
    }
    else if (method == "getSystemInfo") {
        QJsonObject info = systemInfo.collectSystemInfo();
        response["result"] = info;
    }
    else if (method == "getFileSystem") {
        QString path = request.value("params").toObject().value("path").toString();
        QJsonArray arr = fileManager.getFileSystemInfo(path);
        response["result"] = arr;
    }
    else if (method == "getProcessList") {
        QJsonArray arr = processManager.getProcessListAsJsonArray();
        response["result"] = arr;
    }
    else if (method == "getServiceList") {
        QJsonArray arr = serviceManager.getServices();
        response["result"] = arr;
    }
    else if (method == "addUser") {
        QString username = request.value("params").toObject().value("username").toString();
        QString password = request.value("params").toObject().value("password").toString();
        bool ok = userManager.addUser(username, password);
        if (ok) response["result"] = QJsonObject{ {"status", "success"} };
        else {
            QJsonObject err;
            err["code"] = -32001;
            err["message"] = "Failed to add user";
            response["error"] = err;
        }
    }
    else if (method == "removeUser") {
        QString username = request.value("params").toObject().value("username").toString();
        bool ok = userManager.removeUser(username);
        if (ok) response["result"] = QJsonObject{ {"status", "success"} };
        else {
            QJsonObject err;
            err["code"] = -32002;
            err["message"] = "Failed to remove user";
            response["error"] = err;
        }
    }
    else if (method == "changeUserPassword") {
        QString username = request.value("params").toObject().value("username").toString();
        QString password = request.value("params").toObject().value("password").toString();
        bool ok = userManager.changePassword(username, password);
        if (ok) response["result"] = QJsonObject{ {"status", "success"} };
        else {
            QJsonObject err;
            err["code"] = -32003;
            err["message"] = "Failed to change password";
            response["error"] = err;
        }
    }
    else if (method == "setFilePermissions") {
        QString path = request.value("params").toObject().value("path").toString();
        QString perms = request.value("params").toObject().value("permissions").toString();
        bool ok = fileManager.setPermissions(path, perms);
        if (ok) response["result"] = QJsonObject{ {"status", "success"} };
        else {
            QJsonObject err;
            err["code"] = -32004;
            err["message"] = "Failed to set permissions";
            response["error"] = err;
        }
    }
    else if (method == "manageService") {
        QString service = request.value("params").toObject().value("service").toString();
        QString action  = request.value("params").toObject().value("action").toString();
        bool ok = serviceManager.manageService(service, action);
        if (ok) response["result"] = QJsonObject{ {"status", "success"} };
        else {
            QJsonObject err;
            err["code"] = -32005;
            err["message"] = "Failed to manage service";
            response["error"] = err;
        }
    }
    else {
        QJsonObject err;
        err["code"] = -32601; // Method not found
        err["message"] = "Unknown method: " + method;
        response["error"] = err;
    }

    // 7) ���������� ����� (����� length-prefix + JSON)
    sendJsonResponse(client, response);
}

// ������������ ������: 4-����� (BigEndian) ����� + JSON-�ayload
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
