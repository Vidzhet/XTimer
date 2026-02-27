#include "listener.h"
#include <qmessagebox.h>

Listener::Listener()
{
    if (!server.listen("XTimerAPI")) {
        qWarning() << "Unable to start QLocalServer";
    }

    QObject::connect(&server, &QLocalServer::newConnection, [&] {
        auto socket = server.nextPendingConnection();

        QObject::connect(socket, &QLocalSocket::readyRead, [socket] {
            const QByteArray raw = socket->readAll();
            QString input = QString::fromUtf8(raw).trimmed();

            QStringList parts = input.split(' ', Qt::SkipEmptyParts);
            if (parts.isEmpty()) {
                return;
            }
            QString cmd = parts.takeFirst();

            for (auto signal : ListenerRegister::registered) {
                if (cmd == signal.command) {
                    const char* reply = signal.callback(parts);
                    if (reply) {
                        socket->write(reply);
                        socket->flush();
                        delete[] reply;
                    }
                    break;
                }
            }

            });
        });
}

Listener::~Listener()
{
}

ListenerRegister::ListenerRegister(const char* command, const char*(*callback)(const QStringList& args))
{
    registered.append(Signal{ command, callback });
}

Listener* api = nullptr;
