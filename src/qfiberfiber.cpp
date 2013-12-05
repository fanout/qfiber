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

#include "qfiberfiber.h"

#include <assert.h>
#include <lthread.h>
#include <QThread>
#include <QThreadStorage>

// this declaration is missing from lthread.h
extern "C" {
void lthread_exit(void *ptr);
}

namespace QFiber {

enum FuncType
{
	Full,
	NoArgs,
	NoRet,
	NoArgsNoRet,
	MetaMethod
};

class FuncData
{
public:
	FuncType type;
	void *func;
	QObject *obj;
	QByteArray member;
	bool detached;
	QByteArray name;
	QVariantList args;
	QVariant ret;
};

static void wrapfunc(void *arg)
{
	FuncData *f = (FuncData *)arg;

	if(!f->name.isEmpty())
		lthread_set_funcname(f->name.data());

	if(f->detached)
		lthread_detach();

	if(f->type == Full)
		f->ret = ((Func)f->func)(f->args);
	else if(f->type == NoArgs)
		f->ret = ((FuncNoArgs)f->func)();
	else if(f->type == NoRet)
		((FuncNoRet)f->func)(f->args);
	else if(f->type == NoArgsNoRet)
		((FuncNoArgsNoRet)f->func)();
	else // MetaMethod
		invokeMethodWithVariants(f->obj, f->member, f->args, 0, Qt::DirectConnection);

	lthread_exit(&f);
}

class ThreadWorker : public QObject
{
	Q_OBJECT

public:
	FuncData *f;

public slots:
	void start()
	{
		f->detached = true;

		lthread *lt = 0;
		lthread_create(&lt, wrapfunc, f);
		lthread_run();
	}
};

QThreadStorage<bool> lthreadRunning;

class Fiber::Private
{
public:
	QString name;
	bool detached;
	bool threaded;
	lthread_t *lt;

	Private() :
		detached(false),
		threaded(false),
		lt(0)
	{
	}

	~Private()
	{
		if(!detached)
			join(-1);
	}

	void start(FuncData *f)
	{
		f->detached = detached;
		f->name = name.toUtf8();

		if(threaded)
		{
			QThread *thread = new QThread;
			ThreadWorker *worker = new ThreadWorker;
			worker->f = f;
			worker->moveToThread(thread);
			thread->start();
			QMetaObject::invokeMethod(worker, "start", Qt::QueuedConnection);
		}
		else
			lthread_create(&lt, wrapfunc, f);
	}

	QVariant join(int timeout)
	{
		assert(!threaded);
		assert(!detached);

		if(lthreadRunning.localData())
		{
			if(timeout < 0)
				timeout = 0;
			else if(timeout == 0)
				timeout = 1;

			void *ptr;
			lthread_join(lt, &ptr, timeout);
			FuncData *f = (FuncData *)ptr;
			QVariant ret = f->ret;
			delete f;
			lt = 0;
			return ret;
		}
		else
		{
			lthreadRunning.setLocalData(true);
			lthread_run();
			return QVariant();
		}
	}
};

Fiber::Fiber()
{
	d = new Private;
}

Fiber::Fiber(const QString &name)
{
	d = new Private;
	d->name = name;
}

Fiber::~Fiber()
{
	delete d;
}

void Fiber::setName(const QString &name)
{
	d->name = name;
}

void Fiber::setDetached(bool detached)
{
	d->detached = detached;
}

void Fiber::setThreaded(bool threaded)
{
	d->threaded = threaded;
}

void Fiber::start(FuncNoArgs func)
{
	FuncData *f = new FuncData;
	f->type = NoArgs;
	f->func = (void *)func;
	d->start(f);
}

void Fiber::start(FuncNoArgsNoRet func)
{
	FuncData *f = new FuncData;
	f->type = NoArgsNoRet;
	f->func = (void *)func;
	d->start(f);
}

void Fiber::start(Func func, const QVariantList &args)
{
	FuncData *f = new FuncData;
	f->type = Full;
	f->func = (void *)func;
	f->args = args;
	d->start(f);
}

void Fiber::start(FuncNoRet func, const QVariantList &args)
{
	FuncData *f = new FuncData;
	f->type = NoRet;
	f->func = (void *)func;
	f->args = args;
	d->start(f);
}

void Fiber::start(QObject *obj, const char *member, const QVariantList &args)
{
	FuncData *f = new FuncData;
	f->type = MetaMethod;
	f->obj = obj;
	f->member = member;
	f->args = args;
	d->start(f);
}

void Fiber::start(
	Func func,
	const QVariant &arg0,
	const QVariant &arg1,
	const QVariant &arg2,
	const QVariant &arg3,
	const QVariant &arg4,
	const QVariant &arg5,
	const QVariant &arg6,
	const QVariant &arg7,
	const QVariant &arg8,
	const QVariant &arg9)
{
	QVariantList args = variantsToVariantList(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
	start(func, args);
}

void Fiber::start(
	FuncNoRet func,
	const QVariant &arg0,
	const QVariant &arg1,
	const QVariant &arg2,
	const QVariant &arg3,
	const QVariant &arg4,
	const QVariant &arg5,
	const QVariant &arg6,
	const QVariant &arg7,
	const QVariant &arg8,
	const QVariant &arg9)
{
	QVariantList args = variantsToVariantList(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
	start(func, args);
}

void Fiber::start(
	QObject *obj,
	const char *member,
	const QVariant &arg0,
	const QVariant &arg1,
	const QVariant &arg2,
	const QVariant &arg3,
	const QVariant &arg4,
	const QVariant &arg5,
	const QVariant &arg6,
	const QVariant &arg7,
	const QVariant &arg8,
	const QVariant &arg9)
{
	QVariantList args = variantsToVariantList(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
	start(obj, member, args);
}

void Fiber::start(
	QObject *obj,
	const char *member,
	QGenericArgument arg0,
	QGenericArgument arg1,
	QGenericArgument arg2,
	QGenericArgument arg3,
	QGenericArgument arg4,
	QGenericArgument arg5,
	QGenericArgument arg6,
	QGenericArgument arg7,
	QGenericArgument arg8,
	QGenericArgument arg9)
{
	QVariantList args = argumentsToVariantList(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
	start(obj, member, args);
}

QVariant Fiber::join(int timeout)
{
	return d->join(timeout);
}

}

#include "qfiberfiber.moc"
