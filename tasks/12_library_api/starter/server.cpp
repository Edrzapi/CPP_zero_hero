// ============================================================
// server.cpp: Definitions for server.h (starter)
// ============================================================
// TODO Core 2: write serve(). The steps, with the Winsock (Windows)
// and POSIX names, which are nearly the same:
//   1. Windows only: WSAStartup once at the start, WSACleanup at
//      the end (an RAII object does both). Link ws2_32 (done in
//      tasks/CMakeLists.txt).
//   2. socket(AF_INET, SOCK_STREAM, IPPROTO_TCP), bind it to
//      127.0.0.1:port (INADDR_LOOPBACK, htons(port)), listen().
//      Hint: also listen on ::1 (AF_INET6, in6addr_loopback),
//      or clients that try IPv6 first wait 2 seconds a request.
//   3. Call on_ready(), then loop: accept() a connection, read
//      with recv() until "\r\n\r\n", split the request line into
//      method and target, find Content-Length in the headers,
//      read that many bytes of body.
//   4. Call handler(request) and send() back:
//        HTTP/1.1 200 OK\r\n
//        Content-Type: application/json\r\n
//        Content-Length: N\r\n
//        Connection: close\r\n
//        \r\n
//        BODY
//   5. Close the connection (closesocket / close), wrapped in an
//      RAII class so it always happens.
// ============================================================

#include "server.h"

#include <iostream>

void serve(int port, [[maybe_unused]] const Handler& handler, const std::function<void()>& on_ready)
{
    on_ready();
    std::cout << "not implemented: serve() on port " << port << '\n';
}
