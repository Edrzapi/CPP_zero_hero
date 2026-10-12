// ============================================================
// server.h: a Minimal HTTP/1.1 Server on Raw Sockets
// ============================================================
// The C++ standard library has no networking yet, so this talks
// to the operating system directly: Winsock2 on Windows, POSIX
// sockets on Linux and macOS (almost the same API).
//
// It does just enough HTTP for the task: read the request line,
// the headers and Content-Length bytes of body; answer with a
// status line, Content-Type, Content-Length and the body; close
// the connection. One request at a time, so the library is never
// used by two requests at once and needs no lock. (Python's
// ThreadingHTTPServer runs requests on several threads, which is
// why the Python reference guards the library with a lock.)
//
// Real servers handle keep-alive, chunked bodies, timeouts and
// many clients at once; use a library such as Boost.Beast,
// Drogon or cpp-httplib for anything beyond a lesson.
// ============================================================

#pragma once

#include <functional>
#include <string>

struct Request {
    std::string method;   // GET, POST, ...
    std::string path;     // /books (no query string)
    std::string query;    // title=war%20of (still encoded)
    std::string body;
};

struct Response {
    int status = 200;
    std::string body;   // JSON text
};

using Handler = std::function<Response(const Request&)>;

// Listens on localhost (127.0.0.1 and, where available, ::1) and
// answers every request with handler. Runs until the process is
// stopped. on_ready is called once the port is open.
void serve(int port, const Handler& handler, const std::function<void()>& on_ready);
