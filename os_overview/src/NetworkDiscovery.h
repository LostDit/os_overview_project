#ifndef NETWORKDISCOVERY_H
#define NETWORKDISCOVERY_H

#include <QObject>
#include <QUdpSocket>

class NetworkDiscovery : public QObject
{
    Q_OBJECT
public:
    explicit NetworkDiscovery(QObject *parent = nullptr);
    ~NetworkDiscovery();

    void start(quint16 discoveryPort, quint16 tcpPort);

private slots:
    void readPendingDatagrams();

private:
    QUdpSocket *udpSocket;
    quint16 tcpPort_;
};

#endif // NETWORKDISCOVERY_H
