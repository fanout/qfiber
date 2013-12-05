QFiber
======
Date: December 4th, 2013

Author: Justin Karneges <justin@fanout.io>

Mailing List: http://lists.fanout.io/listinfo.cgi/fanout-users-fanout.io

QFiber is a C++ coroutines library that makes it easy to write hybrid event-driven and threaded programs. It uses Lthread for the actual coroutines, ZeroMQ to provide communication between tasks (whether living on the same thread or across threads), and Qt for dynamic typing and implicit sharing of objects.

Tasks communicate over channels which act as pipes for sending value objects. Any QVariant can be sent over a channel. Internally, only object pointers are actually sent, as Qt provides thread-safe data sharing on the heap, meaning that communication over channels is effectively "zero copy".

See:
  * Lthread: https://github.com/halayli/lthread
  * ZeroMQ: http://zeromq.org/
  * Qt dynamic typing: http://qt-project.org/doc/qt-4.8/qvariant.html
  * Qt implicit sharing: http://qt-project.org/doc/qt-4.8/implicit-sharing.html

License
-------

QFiber is offered under the MIT license. See the COPYING file.
