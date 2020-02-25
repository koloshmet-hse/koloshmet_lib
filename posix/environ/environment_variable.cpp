#include "environment_variable.h"

#include <cstdlib>

#include <util/exception/exception.h>

TEnvVar::TEnvVar(std::string key)
    : EnvVar{std::move(key)}
{
    EnvVar += '=';
    if (auto value = std::getenv(EnvVar.c_str()); value != nullptr) {
        EnvVar += value;
    } else {
        throw TException{"No such environment variable"};
    }
}

TEnvVar::TEnvVar(std::string key, std::string_view value)
        : EnvVar{std::move(key)}
{
    EnvVar += '=';
    EnvVar += value;
}

TEnvVar::operator std::string() && noexcept {
    return std::move(EnvVar);
}

TEnvVar::operator std::string() const& {
    return EnvVar;
}

TEnvVar::operator std::string_view() const {
    return EnvVar;
}
