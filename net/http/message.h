#pragma once

#include <istream>
#include <ostream>

#include <unordered_map>

class THttpRequestMessage {
public:
    [[nodiscard]]
    const std::string& GetMethod() const;

    [[nodiscard]]
    const std::string& GetUri() const;

    [[nodiscard]]
    const std::string& GetVersion() const;

    [[nodiscard]]
    const std::string& GetBody() const;

    [[nodiscard]]
    const std::string& GetHeader(const std::string& header) const;

    void SetMethod(std::string method);

    void SetUri(std::string uri);

    void SetVersion(std::string version);

    void SetBody(std::string body);

    void SetHeader(const std::string& header, std::string value);

    std::istream& operator>>(std::istream& stream);

    std::ostream& operator<<(std::ostream& stream);

private:
    std::unordered_map<std::string, std::string> Headers;
    std::string Method;
    std::string Uri;
    std::string Version;
    std::string Body;
};

class THttpResponseMessage {
public:
    [[nodiscard]]
    std::size_t GetStatus() const;

    [[nodiscard]]
    const std::string& GetDescription() const;

    [[nodiscard]]
    const std::string& GetVersion() const;

    [[nodiscard]]
    const std::string& GetBody() const;

    [[nodiscard]]
    const std::string& GetHeader(const std::string& header) const;

    void SetStatus(std::size_t status);

    void SetDescription(std::string description);

    void SetVersion(std::string version);

    void SetBody(std::string body);

    void SetHeader(const std::string& header, std::string value);

    std::istream& operator>>(std::istream& stream);

    std::ostream& operator<<(std::ostream& stream);

private:
    std::unordered_map<std::string, std::string> Headers;
    std::size_t Status;
    std::string Description;
    std::string Version;
    std::string Body;
};