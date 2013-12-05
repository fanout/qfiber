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

#ifndef QFIBERFIBER_H
#define QFIBERFIBER_H

#include <QVariant>
#include "qfiberglobal.h"

namespace QFiber {

class Fiber
{
public:
	Fiber();
	Fiber(const QString &name);
	~Fiber();

	void setName(const QString &name);
	void setDetached(bool detached);
	void setThreaded(bool threaded);

	void start(FuncNoArgs func);
	void start(FuncNoArgsNoRet func);
	void start(Func func, const QVariantList &args);
	void start(FuncNoRet func, const QVariantList &args);
	void start(QObject *obj, const char *member, const QVariantList &args);
	void start(
		Func func,
		const QVariant &arg0,
		const QVariant &arg1 = QVariant(),
		const QVariant &arg2 = QVariant(),
		const QVariant &arg3 = QVariant(),
		const QVariant &arg4 = QVariant(),
		const QVariant &arg5 = QVariant(),
		const QVariant &arg6 = QVariant(),
		const QVariant &arg7 = QVariant(),
		const QVariant &arg8 = QVariant(),
		const QVariant &arg9 = QVariant());
	void start(
		FuncNoRet func,
		const QVariant &arg0,
		const QVariant &arg1 = QVariant(),
		const QVariant &arg2 = QVariant(),
		const QVariant &arg3 = QVariant(),
		const QVariant &arg4 = QVariant(),
		const QVariant &arg5 = QVariant(),
		const QVariant &arg6 = QVariant(),
		const QVariant &arg7 = QVariant(),
		const QVariant &arg8 = QVariant(),
		const QVariant &arg9 = QVariant());
	void start(
		QObject *obj,
		const char *member,
		const QVariant &arg0,
		const QVariant &arg1 = QVariant(),
		const QVariant &arg2 = QVariant(),
		const QVariant &arg3 = QVariant(),
		const QVariant &arg4 = QVariant(),
		const QVariant &arg5 = QVariant(),
		const QVariant &arg6 = QVariant(),
		const QVariant &arg7 = QVariant(),
		const QVariant &arg8 = QVariant(),
		const QVariant &arg9 = QVariant());
	void start(
		QObject *obj,
		const char *member,
		QGenericArgument arg0 = QGenericArgument(),
		QGenericArgument arg1 = QGenericArgument(),
		QGenericArgument arg2 = QGenericArgument(),
		QGenericArgument arg3 = QGenericArgument(),
		QGenericArgument arg4 = QGenericArgument(),
		QGenericArgument arg5 = QGenericArgument(),
		QGenericArgument arg6 = QGenericArgument(),
		QGenericArgument arg7 = QGenericArgument(),
		QGenericArgument arg8 = QGenericArgument(),
		QGenericArgument arg9 = QGenericArgument());

	QVariant join(int timeout = -1);

private:
	Q_DISABLE_COPY(Fiber)

	class Private;
	Private *d;
};

}

#endif
