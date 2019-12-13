#include "message.h"

#include <util/string/utils.h>

#include <util/exception/exception.h>

#include <vector>

const std::string& THttpRequestMessage::GetBody() const {
    return Body;
}

const std::string& THttpRequestMessage::GetMethod() const {
    return Method;
}

const std::string& THttpRequestMessage::GetUri() const {
    return Uri;
}

const std::string& THttpRequestMessage::GetHeader(const std::string& header) const {
    return Headers.at(header);
}

const std::string& THttpRequestMessage::GetVersion() const {
    return Version;
}

void THttpRequestMessage::SetBody(std::string body) {
    Body = std::move(body);
}

void THttpRequestMessage::SetMethod(std::string method) {
    Method = std::move(method);
}

void THttpRequestMessage::SetUri(std::string uri) {
    Uri = std::move(uri);
}

void THttpRequestMessage::SetVersion(std::string version) {
    Version = std::move(version);
}

void THttpRequestMessage::SetHeader(const std::string& header, std::string value) {
    Headers[header] = std::move(value);
}

const std::string& THttpResponseMessage::GetBody() const {
    return Body;
}

std::size_t THttpResponseMessage::GetStatus() const {
    return Status;
}

const std::string& THttpResponseMessage::GetDescription() const {
    return Description;
}

const std::string& THttpResponseMessage::GetHeader(const std::string& header) const {
    return Headers.at(header);
}

const std::string& THttpResponseMessage::GetVersion() const {
    return Version;
}

void THttpResponseMessage::SetBody(std::string body) {
    Body = std::move(body);
}

void THttpResponseMessage::SetStatus(std::size_t status) {
    if (status < 100 || status >= 600) {
        throw TException{"Incorrect http status code"};
    }
    Status = status;
}

void THttpResponseMessage::SetDescription(std::string description) {
    Description = std::move(description);
}

void THttpResponseMessage::SetVersion(std::string version) {
    Version = std::move(version);
}

void THttpResponseMessage::SetHeader(const std::string& header, std::string value) {
    Headers[header] = std::move(value);
}

inline void WriteHeaders(std::ostream& stream, const std::unordered_map<std::string, std::string>& headers) {
    for (auto&& [key, value] : headers) {
        stream << key << ':' << ' ' << value << '\r' << '\n';
    }
}
// TODO: Encoding/Decoding

std::ostream& THttpRequestMessage::operator<<(std::ostream& stream) {
    stream << Method << ' ' << Uri << ' ' << Version << '\r' << '\n';
    WriteHeaders(stream, Headers);
    stream << '\r' << '\n' << Body;
    return stream;
}

std::ostream& THttpResponseMessage::operator<<(std::ostream& stream) {
    stream << Version << ' ' << Status << ' ' << Description << '\r' << '\n';
    WriteHeaders(stream, Headers);
    stream << '\r' << '\n' << Body;
    return stream;
}

std::string_view Prepare(std::string& curLine) {
    if (curLine.back() == '\r') {
        curLine.pop_back();
    }
    return curLine;
}

void ReadHeaders(std::istream& stream, std::string& curLine, std::unordered_map<std::string, std::string>& headers) {
    while (getline(stream, curLine)) {
        if (curLine.size() == 1 && curLine.front() == '\r') {
            break;
        }
        if (curLine.empty()) {
            throw TException{"Empty http request header"};
        }

        std::vector<std::string_view> parts = Split(Prepare(curLine), ": ");
        std::string header(parts.front());
        headers[header].clear();
        for (std::size_t indx = 1; indx < parts.size(); ++indx) {
            headers[header] += parts[indx];
        }
    }
}

inline void ReadBody(std::istream& stream, std::unordered_map<std::string, std::string>& headers, std::string& body) {
    if (auto length = headers.find("Content-Length"); length != headers.end()) {
        auto len = stoul(length->second);
        body.resize(len);
        stream.read(body.data(), len);
    }
}

std::istream& THttpRequestMessage::operator>>(std::istream& stream) {
    std::string curLine;

    getline(stream, curLine);
    if (curLine.empty()) {
        throw TException{"Empty http request start"};
    }
    std::vector<std::string_view> startParts = Split(Prepare(curLine), " ");
    if (startParts.size() != 3) {
        throw TException{"Wrong http request start"};
    }
    Method = startParts[0];
    Uri = startParts[1];
    Version = startParts[2];

    ReadHeaders(stream, curLine, Headers);
    ReadBody(stream, Headers, Body);
    return stream;
}

std::istream& THttpResponseMessage::operator>>(std::istream& stream) {
    std::string curLine;

    getline(stream, curLine);
    if (curLine.empty()) {
        throw TException{"Empty http request start"};
    }
    std::vector<std::string_view> startParts = Split(Prepare(curLine), " ");
    if (startParts.size() >= 2 && startParts.size() <= 3) {
        Version = startParts[0];
        SetStatus(FromString<unsigned long>(startParts[1]));
        if (startParts.size() == 3) {
            Description = startParts[2];
        }
    } else {
        throw TException{"Wrong http request start"};
    }

    ReadHeaders(stream, curLine, Headers);
    ReadBody(stream, Headers, Body);
    return stream;
}
