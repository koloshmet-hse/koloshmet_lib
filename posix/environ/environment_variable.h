#pragma once

#include <string>

class TEnvVar {
    explicit TEnvVar(std::string key);

    explicit TEnvVar(std::string key, std::string_view value);

    [[nodiscard]]
    /* implicit */ operator std::string() && noexcept;

    [[nodiscard]]
    /* implicit */ operator std::string() const&;

    [[nodiscard]]
    /* implicit */ operator std::string_view() const;

private:
    std::string EnvVar;
};
