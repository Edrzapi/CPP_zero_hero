// ============================================================
// server.cpp: Definitions for server.h
// ============================================================

#include "server.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN   // leave the rarely used parts of windows.h out
#define NOMINMAX              // stop windows.h defining min and max macros
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#endif

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace {

// ------------------------------------------------------------
// The Two Socket APIs Side by Side
// ------------------------------------------------------------

#ifdef _WIN32
using socket_t = SOCKET;
constexpr socket_t NO_SOCKET = INVALID_SOCKET;
void close_socket(socket_t s) { closesocket(s); }

// Winsock must be started before the first socket call and
// cleaned up after the last one: a job for an RAII object
struct NetworkSession {
    NetworkSession()
    {
        WSADATA data;
        if (WSAStartup(MAKEWORD(2, 2), &data) != 0) throw std::runtime_error("WSAStartup failed");
    }
    ~NetworkSession() { WSACleanup(); }
};
#else
using socket_t = int;
constexpr socket_t NO_SOCKET = -1;
void close_socket(socket_t s) { close(s); }
struct NetworkSession {};   // nothing to start on POSIX
#endif

// Closes its socket in the destructor, however the scope is left
class Socket {
public:
    explicit Socket(socket_t s = NO_SOCKET) : s_{s} {}
    ~Socket()
    {
        if (s_ != NO_SOCKET) close_socket(s_);
    }
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    Socket(Socket&& other) noexcept : s_{other.s_} { other.s_ = NO_SOCKET; }

    socket_t get() const { return s_; }
    bool valid() const { return s_ != NO_SOCKET; }

private:
    socket_t s_;
};

// ------------------------------------------------------------
// Listening on localhost
// ------------------------------------------------------------
// "localhost" is two addresses: 127.0.0.1 (IPv4) and ::1 (IPv6).
// A client may try either first. Listening on both answers at
// once; listening on 127.0.0.1 alone makes a client that tries
// ::1 first wait for that attempt to fail (about 2 seconds a
// request on Windows).

Socket listen_ipv4(int port)
{
    Socket s{socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)};
    if (!s.valid()) return s;
#ifndef _WIN32
    // Lets a restarted server reuse the port at once. (On Windows
    // SO_REUSEADDR means something riskier, so it is left off.)
    int yes = 1;
    setsockopt(s.get(), SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);
#endif
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(static_cast<unsigned short>(port));   // network byte order
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);              // 127.0.0.1 only, never the network
    if (bind(s.get(), reinterpret_cast<const sockaddr*>(&address), sizeof address) != 0 ||
        listen(s.get(), SOMAXCONN) != 0) {
        return Socket{};
    }
    return s;
}

Socket listen_ipv6(int port)
{
    Socket s{socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP)};
    if (!s.valid()) return s;
    int yes = 1;
    setsockopt(s.get(), IPPROTO_IPV6, IPV6_V6ONLY, reinterpret_cast<const char*>(&yes), sizeof yes);
    sockaddr_in6 address{};
    address.sin6_family = AF_INET6;
    address.sin6_port = htons(static_cast<unsigned short>(port));
    address.sin6_addr = in6addr_loopback;   // ::1
    if (bind(s.get(), reinterpret_cast<const sockaddr*>(&address), sizeof address) != 0 ||
        listen(s.get(), SOMAXCONN) != 0) {
        return Socket{};
    }
    return s;
}

// ------------------------------------------------------------
// Reading a Request, Writing a Response
// ------------------------------------------------------------

constexpr std::size_t MAX_REQUEST = 1024 * 1024;   // refuse anything bigger than 1 MB

// recv returns how many bytes arrived: 0 when the client closed,
// negative on an error (or the timeout set below)
bool receive_more(socket_t s, std::string& buffer)
{
    char chunk[4096];
    const auto got = recv(s, chunk, static_cast<int>(sizeof chunk), 0);
    if (got <= 0) return false;
    buffer.append(chunk, static_cast<std::size_t>(got));
    return buffer.size() <= MAX_REQUEST;
}

std::string lower(std::string_view text)
{
    std::string out;
    for (char c : text) out += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return out;
}

// Reads one request. Returns false if the client sent something
// that is not HTTP, or went quiet.
bool read_request(socket_t s, Request& request)
{
    std::string buffer;
    std::size_t header_end;
    while ((header_end = buffer.find("\r\n\r\n")) == std::string::npos) {
        if (!receive_more(s, buffer)) return false;
    }

    // Request line: METHOD SP TARGET SP VERSION
    const std::size_t line_end = buffer.find("\r\n");
    const std::string line = buffer.substr(0, line_end);
    const std::size_t space1 = line.find(' ');
    const std::size_t space2 = line.find(' ', space1 + 1);
    if (space1 == std::string::npos || space2 == std::string::npos) return false;
    request.method = line.substr(0, space1);
    const std::string target = line.substr(space1 + 1, space2 - space1 - 1);
    const std::size_t question = target.find('?');
    request.path = target.substr(0, question);
    request.query = question == std::string::npos ? "" : target.substr(question + 1);

    // Headers: only Content-Length matters here. Header names are
    // not case-sensitive.
    std::size_t length = 0;
    std::size_t pos = line_end + 2;
    while (pos < header_end) {
        const std::size_t next = buffer.find("\r\n", pos);
        const std::string header = buffer.substr(pos, next - pos);
        const std::size_t colon = header.find(':');
        if (colon != std::string::npos && lower(header.substr(0, colon)) == "content-length") {
            try {
                length = std::stoul(header.substr(colon + 1));
            } catch (const std::exception&) {
                return false;
            }
        }
        pos = next + 2;
    }
    if (length > MAX_REQUEST) return false;

    // The body: Content-Length bytes after the blank line
    const std::size_t body_start = header_end + 4;
    while (buffer.size() < body_start + length) {
        if (!receive_more(s, buffer)) return false;
    }
    request.body = buffer.substr(body_start, length);
    return true;
}

std::string_view reason(int status)
{
    switch (status) {
    case 200: return "OK";
    case 201: return "Created";
    case 400: return "Bad Request";
    case 404: return "Not Found";
    case 409: return "Conflict";
    case 502: return "Bad Gateway";
    default: return "Internal Server Error";
    }
}

void send_all(socket_t s, const std::string& data)
{
    std::size_t sent = 0;
    while (sent < data.size()) {
        const auto n = send(s, data.data() + sent, static_cast<int>(data.size() - sent), 0);
        if (n <= 0) return;   // the client went away: nothing more to do
        sent += static_cast<std::size_t>(n);
    }
}

void send_response(socket_t s, const Response& response)
{
    std::string text = "HTTP/1.1 " + std::to_string(response.status) + " " + std::string(reason(response.status)) +
                       "\r\n";
    text += "Content-Type: application/json; charset=utf-8\r\n";
    text += "Content-Length: " + std::to_string(response.body.size()) + "\r\n";
    text += "Connection: close\r\n\r\n";   // one request per connection
    text += response.body;
    send_all(s, text);
}

void handle_connection(socket_t client, const Handler& handler)
{
    // Give up on a client that sends nothing for 5 seconds
#ifdef _WIN32
    DWORD timeout = 5000;
#else
    timeval timeout{5, 0};
#endif
    setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeout), sizeof timeout);

    Request request;
    if (!read_request(client, request)) {
        send_response(client, {400, R"({"error": "bad request"})"});
        return;
    }
    Response response;
    try {
        response = handler(request);
    } catch (const std::exception& error) {
        std::cerr << "unexpected problem: " << error.what() << '\n';
        response = {500, R"({"error": "unexpected problem"})"};
    }
    std::cerr << request.method << ' ' << request.path << (request.query.empty() ? "" : "?") << request.query
              << " -> " << response.status << '\n';
    send_response(client, response);
}

}  // namespace

void serve(int port, const Handler& handler, const std::function<void()>& on_ready)
{
    // Used only for its constructor and destructor. On POSIX it is
    // an empty struct, so without [[maybe_unused]] GCC and Clang
    // warn that it is never used.
    [[maybe_unused]] NetworkSession network;
    std::vector<Socket> listeners;
    if (Socket s = listen_ipv4(port); s.valid()) listeners.push_back(std::move(s));
    if (Socket s = listen_ipv6(port); s.valid()) listeners.push_back(std::move(s));
    if (listeners.empty()) {
        throw std::runtime_error("cannot listen on port " + std::to_string(port));
    }
    on_ready();

    while (true) {
        // select() waits until one of the listening sockets has a
        // connection ready, so one thread can watch both
        fd_set ready;
        FD_ZERO(&ready);
        socket_t highest = 0;
        for (const Socket& listener : listeners) {
            FD_SET(listener.get(), &ready);
            highest = std::max(highest, listener.get());
        }
        if (select(static_cast<int>(highest) + 1, &ready, nullptr, nullptr, nullptr) <= 0) {
            continue;
        }
        for (const Socket& listener : listeners) {
            if (!FD_ISSET(listener.get(), &ready)) continue;
            Socket client{accept(listener.get(), nullptr, nullptr)};
            if (client.valid()) {
                handle_connection(client.get(), handler);
            }   // client's destructor closes the connection here
        }
    }
}
