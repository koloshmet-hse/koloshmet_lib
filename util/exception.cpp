#include "exception.h"

TException::TException(const TException& exception) noexcept
    : Message{}
    , InternalErrorMessage{exception.InternalErrorMessage}
{
    if (InternalErrorMessage != nullptr) {
        return;
    }

    try {
        Message = exception.Message;
    } catch (const std::exception& exception) {
        InternalErrorMessage = exception.what();
    } catch (...) {
        InternalErrorMessage = "There is something shit happened while copying";
    }
}

TException& TException::operator=(const TException& exception) noexcept {
    InternalErrorMessage = exception.InternalErrorMessage;
    if (InternalErrorMessage != nullptr) {
        return *this;
    }

    try {
        Message = exception.Message;
    } catch (const std::exception& exception) {
        InternalErrorMessage = exception.what();
    } catch (...) {
        InternalErrorMessage = "There is something shit happened while copying";
    }
    return *this;
}

const char* TException::what() const noexcept {
    if (InternalErrorMessage != nullptr) {
        return InternalErrorMessage;
    }

    return Message.c_str();
}
