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

#include "qfiberchannel.h"

#include <assert.h>
#include <QSharedData>
#include <QMutex>
#include <QDataStream>
#include <zmq.h>
#include "qfiberglobal.h"
#include "qfiberzmqmessage.h"
#include "qfiberzmqsocket.h"

namespace QFiber {

class Channel::Private : public QSharedData
{
public:
	QString spec;
	bool inproc;
	ZmqSocket *readEnd;
	ZmqSocket *writeEnd;
	QMutex m;

	Private() :
		readEnd(0),
		writeEnd(0)
	{
	}

	~Private()
	{
		delete writeEnd;
		delete readEnd;
	}

	void ensureInit()
	{
		QMutexLocker locker(&m);

		if(readEnd)
			return;

		readEnd = new ZmqSocket(ZMQ_PAIR);
		writeEnd = new ZmqSocket(ZMQ_PAIR);
		spec.sprintf("inproc://channel-%p", this);
		inproc = true;
		bool ret = readEnd->bind(spec);
		assert(ret);
		writeEnd->connect(spec);
	}

private:
	Q_DISABLE_COPY(Private)
};

Channel::Channel() :
	d(new Private)
{
}

Channel::Channel(const Channel &from) :
	d(from.d)
{
}

Channel::~Channel()
{
}

Channel & Channel::operator=(const Channel &from)
{
	d = from.d;
	return *this;
}

QVariantList Channel::read()
{
	d->ensureInit();

	ZmqMessage message = d->readEnd->read();

	QVariantList params;
	if(d->inproc)
	{
		quintptr tmp;
		memcpy(&tmp, message[0].data(), sizeof(quintptr));
		QVariantList *p = (QVariantList *)tmp;
		params = *p;
		delete p;
	}
	else
	{
		QDataStream ds(message[0]);
		ds >> params;
	}

	return params;
}

QVariant Channel::readSingle()
{
	QVariantList params = read();
	if(params.isEmpty())
		return QVariant();

	return params.first();
}

void Channel::write(const QVariantList &params)
{
	d->ensureInit();

	QByteArray buf;
	if(d->inproc)
	{
		quintptr p = (quintptr)new QVariantList(params);
		buf = QByteArray(sizeof(quintptr), 0);
		memcpy(buf.data(), &p, sizeof(quintptr));
	}
	else
	{
		QDataStream ds(&buf, QIODevice::WriteOnly);
		ds << params;
	}

	d->writeEnd->write(ZmqMessage() << buf);
}

void Channel::write(
	const QVariant &param0,
	const QVariant &param1,
	const QVariant &param2,
	const QVariant &param3,
	const QVariant &param4,
	const QVariant &param5,
	const QVariant &param6,
	const QVariant &param7,
	const QVariant &param8,
	const QVariant &param9)
{
	write(variantsToVariantList(param0, param1, param2, param3, param4,
		param5, param6, param7, param8, param9));
}

void Channel::write(
	QGenericArgument param0,
	QGenericArgument param1,
	QGenericArgument param2,
	QGenericArgument param3,
	QGenericArgument param4,
	QGenericArgument param5,
	QGenericArgument param6,
	QGenericArgument param7,
	QGenericArgument param8,
	QGenericArgument param9)
{
	write(argumentsToVariantList(param0, param1, param2, param3, param4,
		param5, param6, param7, param8, param9));
}

}
