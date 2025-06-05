#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include <QJsonArray>

class ProcessManager
{
public:
    QJsonArray getProcessListAsJsonArray() const;
};

#endif // PROCESSMANAGER_H
