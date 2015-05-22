QFiber
======

Author: Justin Karneges <justin@fanout.io>  
Mailing List: http://lists.fanout.io/listinfo.cgi/fanout-users-fanout.io

QFiber is a C++ coroutines library that makes it easy to write hybrid event-driven and threaded programs. It uses Lthread for the actual coroutines, ZeroMQ to provide communication between tasks (whether living on the same thread or across threads), and Qt for dynamic typing and implicit sharing of objects.

Tasks communicate over channels which act as pipes for sending value objects. Any QVariant can be sent over a channel. Internally, only object pointers are actually sent, as Qt provides thread-safe data sharing on the heap, meaning that communication over channels is effectively "zero copy".

See:
  * Lthread: https://github.com/halayli/lthread
  * ZeroMQ: http://zeromq.org/
  * Qt dynamic typing: http://doc.qt.io/qt-4.8/qvariant.html
  * Qt implicit sharing: http://doc.qt.io/qt-4.8/implicit-sharing.html

License
-------

QFiber is offered under the MIT license. See the COPYING file.

Example
-------

Here's an example of a webserver (adapted from Lthread's own webserver example) that efficiently uses all processors/cores of the machine.

```C++
#include <QCoreApplication>
#include <QThread>
#include <lthread.h>
#include "qfiber.h"

Q_DECLARE_METATYPE(QHostAddress)

void initMeta()
{
    qRegisterMetaType<QHostAddress>();
    qRegisterMetaTypeStreamOperators<QHostAddress>();
}

void handler(const QVariantList &args)
{
    int fd = args[0].toInt();

    QByteArray buf(16384, 0);
    while(true)
    {
        ssize_t ret = lthread_recv(fd, buf.data(), buf.size(), 0, 0);
        if(ret <= 0)
            return;
        if(buf.contains("\r\n\r\n"))
            break;
    }

    QByteArray response = "HTTP/1.0 200 OK\r\nContent-Length: 12\r\n\r\nHello World\n";
    lthread_send(fd, response.data(), response.size(), 0);
    lthread_close(fd);
}

void receiver(const QVariantList &args)
{
    int id = args[0].toInt();
    QFiber::Channel c = qvariant_cast<QFiber::Channel>(args[1]);

    printf("%d: receiver started (thread=%p)\n", id, QThread::currentThread());

    while(true)
    {
        QVariantList params = c.read();
        int fd = params[0].toInt();
        QHostAddress addr = qvariant_cast<QHostAddress>(params[1]);

        // spawn a handler to do the work
        printf("%d: incoming connection from %s\n", id, qPrintable(addr.toString()));
        QFiber::Fiber handlerFiber("handler");
        handlerFiber.setDetached(true);
        handlerFiber.start(handler, fd);
    }
}

void listener(const QVariantList &args)
{
    int port = args[0].toInt();

    QFiber::TcpServer server;
    if(!server.listen(port))
    {
        printf("Failed to bind on port %d\n", port);
        return;
    }

    printf("Starting listener on %d\n", port);

    // create receivers
    int numReceivers = QThread::idealThreadCount();
    QList<QFiber::Channel> receiverChannels;
    for(int n = 0; n < numReceivers; ++n)
    {
        QFiber::Channel c;
        receiverChannels += c;

        QFiber::Fiber receiverFiber(QString("receiver-%1").arg(n));
        receiverFiber.setDetached(true);
        receiverFiber.setThreaded(true);
        receiverFiber.start(receiver, n, QVariant::fromValue(c));
    }

    int r = 0;
    while(true)
    {
        QFiber::TcpServer::ConnectionInfo ci = server.takeNext();
        receiverChannels[r].write(ci.sock, QVariant::fromValue(ci.peerAddress));
        r = (r + 1) % numReceivers;
    }
}

int main(int argc, char **argv)
{
    QCoreApplication qapp(argc, argv);
    initMeta();

    QFiber::Fiber listenerFiber("listener");
    listenerFiber.start(listener, 3128);

    return 0;
}
```
