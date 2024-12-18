#include <QCoreApplication>
#include "Server.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Server MyServer(1111, true); //Create server on port 1111
    for (;;) //Up to 100 times...
    {
        MyServer.ListenForNewConnection(); //Accept new connection (if someones trying to connect)
    }
    return a.exec();
}
