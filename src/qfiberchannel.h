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

#ifndef QFIBERCHANNEL_H
#define QFIBERCHANNEL_H

#include <QVariant>
#include <QExplicitlySharedDataPointer>

namespace QFiber {

class Channel
{
public:
	Channel();
	Channel(const Channel &from);
	virtual ~Channel();
	Channel & operator=(const Channel &from);

	QVariantList read();
	QVariant readSingle();

	void write(
		const QVariant &param0,
		const QVariant &param1 = QVariant(),
		const QVariant &param2 = QVariant(),
		const QVariant &param3 = QVariant(),
		const QVariant &param4 = QVariant(),
		const QVariant &param5 = QVariant(),
		const QVariant &param6 = QVariant(),
		const QVariant &param7 = QVariant(),
		const QVariant &param8 = QVariant(),
		const QVariant &param9 = QVariant());
	void writeArgs(
		QGenericArgument param0,
		QGenericArgument param1 = QGenericArgument(),
		QGenericArgument param2 = QGenericArgument(),
		QGenericArgument param3 = QGenericArgument(),
		QGenericArgument param4 = QGenericArgument(),
		QGenericArgument param5 = QGenericArgument(),
		QGenericArgument param6 = QGenericArgument(),
		QGenericArgument param7 = QGenericArgument(),
		QGenericArgument param8 = QGenericArgument(),
		QGenericArgument param9 = QGenericArgument());
	void write(const QVariantList &params);

private:
	class Private;
	QExplicitlySharedDataPointer<Private> d;
};

}

Q_DECLARE_METATYPE(QFiber::Channel)

#endif
