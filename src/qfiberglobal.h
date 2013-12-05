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

#ifndef QFIBERGLOBAL_H
#define QFIBERGLOBAL_H

#include <QVariant>

namespace QFiber {

typedef QVariant (*Func)(const QVariantList &args);
typedef QVariant (*FuncNoArgs)();
typedef void (*FuncNoRet)(const QVariantList &args);
typedef void (*FuncNoArgsNoRet)();

QByteArray methodReturnType(const QMetaObject *obj, const QByteArray &method, const QList<QByteArray> argTypes);

bool invokeMethodWithVariants(QObject *obj, const QByteArray &method, const QVariantList &args, QVariant *ret, Qt::ConnectionType type);

QVariant argumentToVariant(QGenericArgument arg);

QVariantList argumentsToVariantList(
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

QVariantList variantsToVariantList(
	QVariant arg0 = QVariant(),
	QVariant arg1 = QVariant(),
	QVariant arg2 = QVariant(),
	QVariant arg3 = QVariant(),
	QVariant arg4 = QVariant(),
	QVariant arg5 = QVariant(),
	QVariant arg6 = QVariant(),
	QVariant arg7 = QVariant(),
	QVariant arg8 = QVariant(),
	QVariant arg9 = QVariant());

}

#endif
