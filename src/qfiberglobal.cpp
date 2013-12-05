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

#include "qfiberglobal.h"

#include <QMetaMethod>
#include <QMetaObject>

namespace QFiber {

QByteArray methodReturnType(const QMetaObject *obj, const QByteArray &method, const QList<QByteArray> argTypes)
{
	for(int n = 0; n < obj->methodCount(); ++n)
	{
		QMetaMethod m = obj->method(n);
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
		QByteArray sig = m.methodSignature();
#else
		QByteArray sig = m.signature();
#endif
		int offset = sig.indexOf('(');
		if(offset == -1)
			continue;
		QByteArray name = sig.mid(0, offset);
		if(name != method)
			continue;
		if(m.parameterTypes() != argTypes)
			continue;

		return m.typeName();
	}
	return QByteArray();
}

bool invokeMethodWithVariants(QObject *obj, const QByteArray &method, const QVariantList &args, QVariant *ret, Qt::ConnectionType type)
{
	// QMetaObject::invokeMethod() has a 10 argument maximum
	if(args.count() > 10)
		return false;

	QList<QByteArray> argTypes;
	for(int n = 0; n < args.count(); ++n)
		argTypes += args[n].typeName();

	// get return type
	int metatype = QMetaType::Void;
	QByteArray retTypeName = methodReturnType(obj->metaObject(), method, argTypes);
#if QT_VERSION >= 0x050000
	if(!retTypeName.isEmpty() && retTypeName != "void")
#else
	if(!retTypeName.isEmpty())
#endif
	{
		metatype = QMetaType::type(retTypeName.data());
#if QT_VERSION >= 0x050000
		if(metatype == QMetaType::UnknownType) // lookup failed
#else
		if(metatype == QMetaType::Void) // lookup failed
#endif
			return false;
	}

	QGenericArgument arg[10];
	for(int n = 0; n < args.count(); ++n)
		arg[n] = QGenericArgument(args[n].typeName(), args[n].constData());

	QGenericReturnArgument retarg;
	QVariant retval;

	if(metatype != QMetaType::Void)
	{
		retval = QVariant(metatype, (const void *)0);
		retarg = QGenericReturnArgument(retval.typeName(), retval.data());
	}

	if(!QMetaObject::invokeMethod(obj, method.data(), type, retarg, arg[0], arg[1], arg[2], arg[3], arg[4], arg[5], arg[6], arg[7], arg[8], arg[9]))
		return false;

	if(retval.isValid() && ret)
		*ret = retval;
	return true;
}

QVariant argumentToVariant(QGenericArgument arg)
{
	return QVariant(QMetaType::type(arg.name()), arg.data());
}

#define TRY_ARG(arg) if(!arg.data()) { return args; } args += argumentToVariant(arg);

QVariantList argumentsToVariantList(
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
	QVariantList args;
	TRY_ARG(arg0)
	TRY_ARG(arg1)
	TRY_ARG(arg2)
	TRY_ARG(arg3)
	TRY_ARG(arg4)
	TRY_ARG(arg5)
	TRY_ARG(arg6)
	TRY_ARG(arg7)
	TRY_ARG(arg8)
	TRY_ARG(arg9)
	return args;
}

#define TRY_VARG(arg) if(!arg.isValid()) { return args; } args += arg;

QVariantList variantsToVariantList(
	QVariant arg0,
	QVariant arg1,
	QVariant arg2,
	QVariant arg3,
	QVariant arg4,
	QVariant arg5,
	QVariant arg6,
	QVariant arg7,
	QVariant arg8,
	QVariant arg9)
{
	QVariantList args;
	TRY_VARG(arg0)
	TRY_VARG(arg1)
	TRY_VARG(arg2)
	TRY_VARG(arg3)
	TRY_VARG(arg4)
	TRY_VARG(arg5)
	TRY_VARG(arg6)
	TRY_VARG(arg7)
	TRY_VARG(arg8)
	TRY_VARG(arg9)
	return args;
}

}
