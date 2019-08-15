#include <exception>

#include <sstream>
#include <string_view>

class TException : std::exception {
public:
    template <typename... TArgs>
    explicit TException(TArgs&&... args)
        : Message{}
        , InternalErrorMessage{nullptr}
    {
        std::ostringstream stream;
        (stream << ... << std::forward<TArgs>(args));
        Message = stream.str();
    }

    TException(const TException& exception) noexcept;

    TException& operator=(const TException& exception) noexcept;

    const char* what() const noexcept override;

private:
    std::string Message;
    const char* InternalErrorMessage;
};
