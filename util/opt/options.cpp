#include "options.h"

#include <util/string/utils.h>

std::size_t TOptions::Size() const {
    return Parameters.size();
}

void TOptions::SetValue(TOptions::TElement& elem, std::string_view str) {
    auto visitor = [str](auto&& var) {
        using TType = std::decay_t<decltype(var)>;
        if constexpr (!std::is_same_v<TType, bool>) {
            var = FromString<TType>(str);
        } else {
            var = true;
        }
    };
    visit(visitor, elem);
}

std::vector<std::string_view> TOptions::InsertArgs(std::size_t argc, char* argv[]) {
    std::vector<std::string_view> otherParams;
    std::size_t paramsFilled = 0;
    for (std::size_t i = 0; i < argc; ++i) {
        std::string_view arg = argv[i];
        std::string_view key;
        std::string_view value;
        if (arg.substr(0, 2) == "--") {
            std::size_t keyEnd = arg.find('=');
            if (keyEnd != std::string_view::npos) {
                value = arg.substr(keyEnd + 1);
                keyEnd -= 2;
            }
            key = arg.substr(2, keyEnd);
        } else if (arg.front() == '-') {
            key = arg.substr(1, 1);
            std::size_t valStart = 2;
            if (arg.find('=') == 2) {
                ++valStart;
            }
            value = arg.substr(valStart);
        } else {
            if (paramsFilled < Parameters.size()) {
                SetValue(Parameters.at(paramsFilled++), arg);
            } else {
                otherParams.push_back(arg);
            }
            continue;
        }

        if (value.empty() && Options.at(key).index() != TElementTypes::IndexOf<bool>() && i < argc - 1) {
            value = argv[++i];
        }

        SetValue(Options.at(key), value);
    }
    return otherParams;
}

void TOptions::WrapHelpParam(std::ostream& stream, std::string_view str) {
    stream << ' ' << str;
}
