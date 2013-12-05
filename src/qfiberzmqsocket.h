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

#ifndef QFIBERZMQSOCKET_H
#define QFIBERZMQSOCKET_H

#include <QString>
#include "qfiberzmqmessage.h"

namespace QFiber {

class ZmqContext;

class ZmqSocket
{
public:
	ZmqSocket(int ztype, ZmqContext *context = 0);
	~ZmqSocket();

	void connect(const QString &spec);
	bool bind(const QString &spec);

	ZmqMessage read();
	void write(const ZmqMessage &message);

	// the zmq socket
	void *socket() { return sock_; }

private:
	Q_DISABLE_COPY(ZmqSocket)

	void *sock_;
	int fd_;
	ZmqContext *context_;
	bool usingGlobalContext_;
};

}

#endif
