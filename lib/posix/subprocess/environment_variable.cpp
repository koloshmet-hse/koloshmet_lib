#include "environment_variable.h"

#include <cstdlib>

#include <util/exception/exception.h>

NSubprocess::TEnvVar::TEnvVar(std::string key)
    : EnvVar{std::move(key)}
{
    if (auto value = std::getenv(EnvVar.c_str()); value != nullptr) {
        EnvVar += '=';
        EnvVar += value;
    } else {
        throw TException{"No such environment variable"};
    }
}

NSubprocess::TEnvVar::TEnvVar(std::string key, std::string_view value)
        : EnvVar{std::move(key)}
{
    EnvVar += '=';
    EnvVar += value;
}

NSubprocess::TEnvVar::operator std::string() && noexcept {
    return std::move(EnvVar);
}

NSubprocess::TEnvVar::operator std::string() const& {
    return EnvVar;
}

NSubprocess::TEnvVar::operator std::string_view() const {
    return EnvVar;
}
