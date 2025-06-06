#include "server.h"
#include "networkdiscovery.h"
#include <QCoreApplication>

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    Server server;
    if(!server.start(12345)) {
        return 1;
    }

    // Запуск обнаружения на UDP порту 45454
    server.startDiscovery(45454, 12345);

    return a.exec();
}
