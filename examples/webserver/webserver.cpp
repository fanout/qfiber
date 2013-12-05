#include <QCoreApplication>
#include <QThread>
#include <lthread.h>
#include "qfiber.h"

Q_DECLARE_METATYPE(QHostAddress)

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

	qRegisterMetaType<QFiber::ZmqMessage>();
	qRegisterMetaType<QFiber::Channel>();
	qRegisterMetaType<QHostAddress>();
	qRegisterMetaTypeStreamOperators<QHostAddress>();

	QFiber::Fiber listenerFiber("listener");
	listenerFiber.start(listener, 3128);

	return 0;
}
