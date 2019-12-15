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

bool THttpRequestMessage::ContainsHeader(const std::string& header) const {
    return Headers.find(header) != Headers.end();
}

const std::unordered_map<std::string, std::string>& THttpRequestMessage::GetAllHeaders() const {
    return Headers;
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

bool THttpResponseMessage::ContainsHeader(const std::string& header) const {
    return Headers.find(header) != Headers.end();
}

const std::unordered_map<std::string, std::string>& THttpResponseMessage::GetAllHeaders() const {
    return Headers;
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

void WriteHeaders(std::ostream& stream, const std::unordered_map<std::string, std::string>& headers) {
    for (auto&& [key, value] : headers) {
        stream << key << ':' << ' ' << value << '\r' << '\n';
    }
}
// TODO: Encoding/Decoding

std::ostream& operator<<(std::ostream& stream, const THttpRequestMessage& message) {
    stream << message.GetMethod() << ' ' << message.GetUri() << ' ' << message.GetVersion() << '\r' << '\n';
    WriteHeaders(stream, message.GetAllHeaders());
    stream << '\r' << '\n' << message.GetBody();
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const THttpResponseMessage& message) {
    stream << message.GetVersion() << ' ' << message.GetStatus() << ' ' << message.GetDescription() << '\r' << '\n';
    WriteHeaders(stream, message.GetAllHeaders());
    stream << '\r' << '\n' << message.GetBody();
    return stream;
}

std::string_view Prepare(std::string& curLine) {
    if (curLine.back() == '\r') {
        curLine.pop_back();
    }
    return curLine;
}

template <typename TMessage>
void ReadHeaders(std::istream& stream, std::string& curLine, TMessage& message) {
    while (getline(stream, curLine)) {
        if (curLine.size() == 1 && curLine.front() == '\r') {
            break;
        }
        if (curLine.empty()) {
            throw TException{"Empty http request header"};
        }

        std::vector<std::string_view> parts = Split(Prepare(curLine), ": ");
        std::string header(parts.front());
        std::string newHeaderValue;
        for (std::size_t indx = 1; indx < parts.size(); ++indx) {
            newHeaderValue += parts[indx];
        }
        message.SetBody(std::move(newHeaderValue));
    }
}

template <typename TMessage>
void ReadBody(std::istream& stream, TMessage& message) {
    if (message.ContainsHeader("Content-Length")) {
        std::string body;
        body.resize(stoul(message.GetHeader("Content-Length")));
        stream.read(body.data(), body.size());
    }
}

std::istream& operator>>(std::istream& stream, THttpRequestMessage& message) {
    std::string curLine;

    getline(stream, curLine);
    if (curLine.empty()) {
        throw TException{"Empty http request start"};
    }
    std::vector<std::string_view> startParts = Split(Prepare(curLine), " ");
    if (startParts.size() != 3) {
        throw TException{"Wrong http request start"};
    }
    message.SetMethod(std::string{startParts[0]});
    message.SetUri(std::string{startParts[1]});
    message.SetVersion(std::string{startParts[2]});

    ReadHeaders(stream, curLine, message);
    ReadBody(stream, message);
    return stream;
}

std::istream& operator>>(std::istream& stream, THttpResponseMessage& message) {
    std::string curLine;

    getline(stream, curLine);
    if (curLine.empty()) {
        throw TException{"Empty http request start"};
    }
    std::vector<std::string_view> startParts = Split(Prepare(curLine), " ");
    if (startParts.size() >= 2 && startParts.size() <= 3) {
        message.SetVersion(std::string{startParts[0]});
        message.SetStatus(FromString<unsigned long>(startParts[1]));
        if (startParts.size() == 3) {
            message.SetDescription(std::string{startParts[2]});
        }
    } else {
        throw TException{"Wrong http request start"};
    }

    ReadHeaders(stream, curLine, message);
    ReadBody(stream, message);
    return stream;
}
