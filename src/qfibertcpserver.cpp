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

#include "qfibertcpserver.h"

#include <lthread.h>

namespace QFiber {

TcpServer::TcpServer()
{
}

TcpServer::~TcpServer()
{
}

bool TcpServer::listen(int port)
{
	// create listening socket
	fd = lthread_socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(fd == -1)
		return false;

	int opt = 1;
	if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) == -1)
		perror("failed to set SOREUSEADDR on socket");

	sockaddr_in sin;
	sin.sin_family = PF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(port);

	// bind to the listening port
	int ret = bind(fd, (struct sockaddr *)&sin, sizeof(sin));
	if(ret == -1)
		return false;

	::listen(fd, 128);
	return true;
}

TcpServer::ConnectionInfo TcpServer::takeNext()
{
	sockaddr_in peer_addr;
	socklen_t addrlen = sizeof(peer_addr);

	// block until a new connection arrives
	//printf("accept: %d, %d\n", fd, addrlen);
	int clientFd = lthread_accept(fd, (sockaddr *)&peer_addr, &addrlen);
	if(clientFd == -1)
	{
		perror("Failed to accept connection");
		abort();
		return ConnectionInfo();
	}

	ConnectionInfo ci;
	ci.sock = clientFd;
	ci.peerAddress = QHostAddress((sockaddr *)&peer_addr);
	return ci;
}

}
