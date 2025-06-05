#include "server.h"
#include <QCoreApplication>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    Server server(nullptr);
    server.start(12345);
    server.startDiscovery(45454, 12345);

    return app.exec();
}
