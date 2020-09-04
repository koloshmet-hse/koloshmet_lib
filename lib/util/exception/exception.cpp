#include "exception.h"

TException::TException(const TException& exception) noexcept
    : Owner{}
    , Message{exception.Message}
{
    if (Message != nullptr) {
        return;
    }

    try {
        Owner = exception.Owner;
    } catch (const std::exception& exception) {
        Message = exception.what();
    } catch (...) {
        Message = "There is something shit happened while copying";
    }
}

TException& TException::operator=(const TException& exception) noexcept {
    Message = exception.Message;
    if (Message != nullptr) {
        return *this;
    }

    try {
        Owner = exception.Owner;
    } catch (const std::exception& exception) {
        Message = exception.what();
    } catch (...) {
        Message = "There is something shit happened while copying";
    }
    return *this;
}

const char* TException::what() const noexcept {
    if (Message != nullptr) {
        return Message;
    }

    return Owner.c_str();
}
