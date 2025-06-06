#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include "networkdiscovery.h"
#include "filemanager.h"
#include "usermanager.h"
#include "servicemanager.h"
#include "systeminfo.h"
#include "processmanager.h"

class Server : public QTcpServer {
    Q_OBJECT
public:
    explicit Server(QObject* parent = nullptr);
    ~Server();

    bool start(quint16 port);
    void startDiscovery(quint16 discoveryPort, quint16 tcpPort); // <--- Добавляем

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void onClientReadyRead();
    void handleClientDisconnected();

private:
    void sendJsonResponse(QTcpSocket* client, const QJsonObject& response);
    void handleUploadFile(QTcpSocket* client, const QJsonObject& params, int id);
    void handleDownloadFile(QTcpSocket* client, const QJsonObject& params, int id);


    NetworkDiscovery discovery;
    FileManager fileManager;
    UserManager userManager;
    ServiceManager serviceManager;
    SystemInfo systemInfo;
    ProcessManager processManager;

    QMap<QTcpSocket*, QByteArray> clientBuffers;
    QMap<QTcpSocket*, quint32> clientBlockSizes;
};

#endif // SERVER_H

