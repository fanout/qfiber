/*
 * Copyright (C) 2013 Fanout, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "qfiberzmqsocket.h"

#include <assert.h>
#include <QMutex>
#include <lthread.h>
#include <zmq.h>
#include "qfiberzmqcontext.h"

// needed for _lthread_sched_event
extern "C" {
#include "lthread_int.h"
}

static bool get_rcvmore(void *sock)
{
	qint64 more;
	size_t opt_len = sizeof(more);
	int ret = zmq_getsockopt(sock, ZMQ_RCVMORE, &more, &opt_len);
	assert(ret == 0);
	return more ? true : false;
}

static int get_fd(void *sock)
{
	int fd;
	size_t opt_len = sizeof(fd);
	int ret = zmq_getsockopt(sock, ZMQ_FD, &fd, &opt_len);
	assert(ret == 0);
	return fd;
}

static int get_events(void *sock)
{
	quint32 events;
	size_t opt_len = sizeof(events);
	int ret = zmq_getsockopt(sock, ZMQ_EVENTS, &events, &opt_len);
	assert(ret == 0);
	return (int)events;
}

Q_GLOBAL_STATIC(QMutex, g_mutex)

namespace QFiber {

class Global
{
public:
	ZmqContext context;
	int refs;

	Global() :
			refs(0)
	{
	}
};

static Global *global = 0;

static ZmqContext *addGlobalContextRef()
{
	QMutexLocker locker(g_mutex());

	if(!global)
		global = new Global;

	++(global->refs);
	return &(global->context);
}

static void removeGlobalContextRef()
{
	QMutexLocker locker(g_mutex());

	assert(global);
	assert(global->refs > 0);

	--(global->refs);
	if(global->refs == 0)
	{
		delete global;
		global = 0;
	}
}

ZmqSocket::ZmqSocket(int ztype, ZmqContext *context)
{
	if(context)
	{
		usingGlobalContext_ = false;
		context_ = context;
	}
	else
	{
		usingGlobalContext_ = true;
		context_ = addGlobalContextRef();
	}

	sock_ = zmq_socket(context_->context(), ztype);
	fd_ = get_fd(sock_);
}

ZmqSocket::~ZmqSocket()
{
	zmq_close(sock_);

	if(usingGlobalContext_)
		removeGlobalContextRef();
}

void ZmqSocket::connect(const QString &spec)
{
	int ret = zmq_connect(sock_, spec.toUtf8().data());
	assert(ret == 0);
}

bool ZmqSocket::bind(const QString &spec)
{
	int ret = zmq_bind(sock_, spec.toUtf8().data());
	if(ret != 0)
		return false;

	return true;
}

ZmqMessage ZmqSocket::read()
{
	ZmqMessage in;

	while(true)
	{
		zmq_msg_t msg;
		int ret = zmq_msg_init(&msg);
		assert(ret == 0);
		while(true)
		{
			ret = zmq_recv(sock_, &msg, ZMQ_NOBLOCK);
			if(ret < 0 && errno == EAGAIN)
			{
				int flags = 0;
				while(!(flags & ZMQ_POLLIN))
				{
					_lthread_sched_event(lthread_current(), fd_, LT_EV_READ, 0);
					flags = get_events(sock_);
				}
			}
			else
			{
				assert(ret == 0);
				break;
			}
		}
		QByteArray buf((const char *)zmq_msg_data(&msg), zmq_msg_size(&msg));
		ret = zmq_msg_close(&msg);
		assert(ret == 0);
		in += buf;
		if(!get_rcvmore(sock_))
			break;
	}

	return in;
}

void ZmqSocket::write(const ZmqMessage &message)
{
	for(int n = 0; n < message.count(); ++n)
	{
		const QByteArray &buf = message[n];
		zmq_msg_t msg;
		int ret = zmq_msg_init_size(&msg, buf.size());
		assert(ret == 0);
		memcpy(zmq_msg_data(&msg), buf.data(), buf.size());
		while(true)
		{
			ret = zmq_send(sock_, &msg, ZMQ_NOBLOCK | (n + 1 < message.count() ? ZMQ_SNDMORE : 0));
			if(ret < 0 && errno == EAGAIN)
			{
				int flags = 0;
				while(!(flags & ZMQ_POLLOUT))
				{
					_lthread_sched_event(lthread_current(), fd_, LT_EV_WRITE, 0);
					flags = get_events(sock_);
				}
			}
			else
			{
				assert(ret == 0);
				break;
			}
		}
		ret = zmq_msg_close(&msg);
		assert(ret == 0);
	}
}

}
