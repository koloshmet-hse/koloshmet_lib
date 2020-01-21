#pragma once

#include <util/opt/command.h>

#include <filesystem>

class TOptions {
public:
    template <typename... TOpts, typename... TParams, typename TParam>
    TOptions(
        int argc, char* argv[],
        TParamList<TParams...> params, TDefaultParam<TParam> par, const TOpt<TOpts>&... options)
        : Command{std::filesystem::path{GetZeroArg(argc, argv)}.filename().string(), "", params, par, options...}
        , Executable{GetZeroArg(argc, argv)}
    {
        Command.Init(argc - 1, argv + 1);
    }

    template <typename... TOpts, typename... TParams>
    TOptions(
        int argc, char* argv[],
        TParamList<TParams...> params, const TOpt<TOpts>&... options)
        : Command{std::filesystem::path{GetZeroArg(argc, argv)}.filename().string(), "", params, options...}
        , Executable{GetZeroArg(argc, argv)}
    {
        Command.Init(argc - 1, argv + 1);
    }

    TOptions(int argc, char* argv[], std::initializer_list<TCommand> lst);

    template <typename T>
    [[nodiscard]]
    const T& Get(std::string_view opt) const {
        return Command.Get<T>(opt);
    }

    template <typename T>
    [[nodiscard]]
    const T& Get(std::size_t opt) const {
        return Command.Get<T>(opt);
    }

    [[nodiscard]]
    std::size_t Size() const;

    [[nodiscard]]
    std::string_view GetCommand() const;

    [[nodiscard]]
    const std::filesystem::path& GetExecutable() const;

private:
    static std::string_view GetZeroArg(int argc, char* argv[]);

private:
    TCommand Command;
    std::filesystem::path Executable;
};
