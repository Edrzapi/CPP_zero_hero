// ============================================================
// openlibrary.cpp: Definitions for openlibrary.h
// ============================================================
// The HTTP call is made by the system's curl program, through
// popen: it runs a command and hands back what the command
// prints. curl.exe ships with Windows 10 and later, macOS and
// Linux, so no library is needed. (WinHTTP is a Windows-only
// alternative; libcurl the usual library choice.)
// ============================================================

#include "openlibrary.h"

#include "json.h"

#include <cstddef>
#include <cstdio>
#include <format>

#ifdef _WIN32
#define popen _popen     // the same functions with a leading underscore
#define pclose _pclose
#endif

namespace {

// Everything except letters, digits and -._~ becomes %XX, and a
// space becomes +. The result contains no quotes, spaces or shell
// characters, so it is safe inside the double-quoted URL below.
// (cmd.exe could read %name% as a variable, but only one that
// exists; nothing in a URL-encoded title looks like one.)
std::string url_encode(std::string_view text)
{
    std::string out;
    for (const char c : text) {
        const auto byte = static_cast<unsigned char>(c);
        if ((byte >= 'A' && byte <= 'Z') || (byte >= 'a' && byte <= 'z') || (byte >= '0' && byte <= '9') ||
            c == '-' || c == '.' || c == '_' || c == '~') {
            out += c;
        } else if (c == ' ') {
            out += '+';
        } else {
            out += std::format("%{:02X}", static_cast<unsigned>(byte));
        }
    }
    return out;
}

// The member's text, or fallback when it is missing or not text
std::string text_or(const JsonValue& doc, std::string_view key, const std::string& fallback)
{
    const JsonValue* value = doc.find(key);
    return value != nullptr && value->kind == JsonValue::Kind::text ? value->text : fallback;
}

std::string number_or(const JsonValue& doc, std::string_view key, const std::string& fallback)
{
    const JsonValue* value = doc.find(key);
    return value != nullptr && value->kind == JsonValue::Kind::integer ? std::to_string(value->integer) : fallback;
}

}  // namespace

std::string search_open_library(const std::string& title, int limit)
{
    const std::string url = "https://openlibrary.org/search.json?title=" + url_encode(title) +
                            "&limit=" + std::to_string(limit) +
                            "&fields=key,title,author_name,first_publish_year,edition_count";
    // -s silent, -f fail on an HTTP error, --max-time 10 seconds in all
    const std::string command =
        "curl -s -f --max-time 10 -A \"ZeroToHeroLibrary/1.0 (teaching example)\" \"" + url + "\"";

#ifdef _WIN32
    FILE* pipe = popen(command.c_str(), "rb");   // b: no \r\n translation
#else
    FILE* pipe = popen(command.c_str(), "r");
#endif
    if (pipe == nullptr) throw OpenLibraryError("cannot run curl");

    std::string text;
    char buffer[4096];
    std::size_t got;
    while ((got = std::fread(buffer, 1, sizeof buffer, pipe)) > 0) {
        text.append(buffer, got);
    }
    // pclose waits for curl to finish and returns its exit status:
    // anything but 0 is a failure (no network, a timeout, a 404, ...)
    if (pclose(pipe) != 0 || text.empty()) {
        throw OpenLibraryError("Open Library did not answer");
    }
    return text;
}

// Any field can be missing from a real response, so every read
// has a fallback.
std::vector<std::string> summarise(std::string_view json_text)
{
    const JsonValue document = parse_json(json_text);
    std::vector<std::string> lines{"Open Library search: " + number_or(document, "numFound", "0") + " results"};
    const JsonValue* docs = document.find("docs");
    if (docs == nullptr || docs->kind != JsonValue::Kind::array) return lines;

    int number = 0;
    for (const JsonValue& doc : docs->items) {
        std::string authors;
        if (const JsonValue* names = doc.find("author_name"); names != nullptr) {
            for (const JsonValue& name : names->items) {
                if (name.kind != JsonValue::Kind::text) continue;
                authors += (authors.empty() ? "" : ", ") + name.text;
            }
        }
        lines.push_back(std::to_string(++number) + ". " + text_or(doc, "title", "untitled") + " by " +
                        (authors.empty() ? "unknown author" : authors) + ", first published " +
                        number_or(doc, "first_publish_year", "unknown") + ", " +
                        number_or(doc, "edition_count", "0") + " editions, " + text_or(doc, "key", "no key"));
    }
    return lines;
}

std::string first_match(std::string_view json_text)
{
    const JsonValue document = parse_json(json_text);
    const JsonValue* docs = document.find("docs");
    if (docs == nullptr || docs->kind != JsonValue::Kind::array || docs->items.empty()) {
        return "null";
    }
    const JsonValue& doc = docs->items.front();
    // to_json of a missing field is null; authors is [] when missing
    auto field = [&doc](std::string_view key) {
        const JsonValue* value = doc.find(key);
        return value == nullptr ? std::string("null") : to_json(*value);
    };
    const JsonValue* authors = doc.find("author_name");
    return "{\"key\": " + field("key") + ", \"title\": " + field("title") +
           ", \"authors\": " + (authors == nullptr ? std::string("[]") : to_json(*authors)) +
           ", \"first_publish_year\": " + field("first_publish_year") +
           ", \"edition_count\": " + field("edition_count") + "}";
}
