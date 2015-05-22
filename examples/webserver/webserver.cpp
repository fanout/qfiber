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
	QFiber::Channel c = qvariant_cast<QFiber::Channel>(args[1]);

	bool quit = false;
	QByteArray buf(16384, 0);
	while(true)
	{
		ssize_t ret = lthread_recv(fd, buf.data(), buf.size(), 0, 0);
		if(ret <= 0)
			return;
		if(buf.contains("/quit"))
			quit = true;
		if(buf.contains("\r\n\r\n"))
			break;
	}

	if(quit)
	{
		QByteArray response = "HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 9\r\n\r\nQuitting\n";
		lthread_send(fd, response.data(), response.size(), 0);
		lthread_close(fd);
		c.write("quit");
	}
	else
	{
		QByteArray response = "HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 12\r\n\r\nHello World\n";
		lthread_send(fd, response.data(), response.size(), 0);
		lthread_close(fd);
	}
}

void receiver(const QVariantList &args)
{
	int id = args[0].toInt();
	QFiber::Channel lc = qvariant_cast<QFiber::Channel>(args[1]);
	QFiber::Channel lwc = qvariant_cast<QFiber::Channel>(args[2]);

	printf("%d: receiver started (thread=%p)\n", id, QThread::currentThread());
	QFiber::Channel hc;

	QFiber::Poller poller;
	poller.addRead(lc);
	poller.addRead(hc);

	while(true)
	{
		QFiber::Channel c = poller.poll();

		if(c == lc)
		{
			QVariantList params = lc.read();
			int fd = params[0].toInt();
			QHostAddress addr = qvariant_cast<QHostAddress>(params[1]);

			// spawn a handler to do the work
			printf("%d: incoming connection from %s\n", id, qPrintable(addr.toString()));
			QFiber::Fiber handlerFiber("handler");
			handlerFiber.setDetached(true);
			handlerFiber.start(handler, fd, QVariant::fromValue(hc));
		}
		else // hc
		{
			QString command = hc.readSingle().toString();
			if(command == "quit")
				break;
		}
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

	QFiber::Poller poller;
	poller.addRead(server);

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
