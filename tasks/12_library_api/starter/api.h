// ============================================================
// api.h: the Library over HTTP, JSON In and Out (starter)
// ============================================================
// LibraryApi knows nothing about sockets: it takes a Request and
// returns a Response, so it can be tested without a network.
// ============================================================

#pragma once

#include "library.h"
#include "server.h"

class LibraryApi {
public:
    LibraryApi(Library library, Date today);

    Response handle(const Request& request);

private:
    Library library_;
    Date today_;
};
