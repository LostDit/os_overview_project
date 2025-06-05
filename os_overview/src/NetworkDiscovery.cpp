#include "networkdiscovery.h"
#include <QNetworkDatagram>
#include <QHostAddress>
#include <QDebug>

NetworkDiscovery::NetworkDiscovery(QObject* parent)
    : QObject(parent), udpSocket(new QUdpSocket(this)), tcpPort_(0)
{ }

NetworkDiscovery::~NetworkDiscovery() {
    // udpSocket удалится автоматически как дочерний QObject
}

void NetworkDiscovery::start(quint16 discoveryPort, quint16 tcpPort) {
    tcpPort_ = tcpPort;
    // Привязываемся (bind) на UDP-порт discoveryPort
    if (!udpSocket->bind(QHostAddress::AnyIPv4, discoveryPort,
                         QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) {
        qWarning() << "Discovery: could not bind to port" << discoveryPort;
        return;
    }
    connect(udpSocket, &QUdpSocket::readyRead, this, &NetworkDiscovery::readPendingDatagrams);
    qInfo() << "Discovery started on UDP port" << discoveryPort;
}

void NetworkDiscovery::readPendingDatagrams() {
    while (udpSocket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = udpSocket->receiveDatagram();
        QByteArray data = datagram.data();

        // Если пришла строка "DISCOVER_OS_OVERVIEW" → отвечаем "OS_OVERVIEW:<TCP-PORT>"
        if (data == "DISCOVER_OS_OVERVIEW") {
            QByteArray response = "OS_OVERVIEW:" + QByteArray::number(tcpPort_);
            udpSocket->writeDatagram(response,
                                     datagram.senderAddress(),
                                     datagram.senderPort());
            qInfo() << "Discovery: answered" << datagram.senderAddress().toString()
                    << "port" << datagram.senderPort() << "->" << response;
        }
        else {
            // Если пришло что-то другое — игнорируем
        }
    }
}
