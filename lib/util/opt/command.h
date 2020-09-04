#pragma once

#include <lib/util/opt/opt.h>

#include <variant>
#include <unordered_map>
#include <vector>

#include <string_view>

#include <lib/util/exception/exception.h>

#include <iomanip>
#include <iostream>

class TCommand {
    using TElement = std::variant<std::string_view, long double, long long, bool>;
    using TElementTypes = TTypeList<std::string_view, long double, long long, bool>;

public:
    TCommand() = default;

    template <typename... TOpts, typename... TParams, typename TParam>
    TCommand(
        std::string_view name, std::string_view description,
        TParamList<TParams...> params, TDefaultParam<TParam> par, const TOpt<TOpts>&... options)
        : Options{}
        , Parameters(sizeof...(TParams))
        , Default{TElement{std::in_place_type_t<TParam>{}}}
        , Help(ConfigureHelp(name, params, par, options...))
        , Name{name}
        , Description{description}
    {
        Options["help"].emplace<bool>(false);
        (Options[options.Key].template emplace<TOpts>(options.Default), ...);
        ParamsInit<TParams...>(std::index_sequence_for<TParams...>{});
    }

    template <typename... TOpts, typename... TParams>
    TCommand(
        std::string_view name, std::string_view description,
        TParamList<TParams...> params, const TOpt<TOpts>&... options)
        : Options{}
        , Parameters(sizeof...(TParams))
        , Default{}
        , Help(ConfigureHelp(name, params, options...))
        , Name{name}
        , Description{description}
    {
        Options["help"].emplace<bool>(false);
        (Options[options.Key].template emplace<TOpts>(options.Default), ...);
        ParamsInit<TParams...>(std::index_sequence_for<TParams...>{});
    }

    void Init(int argc, char* argv[]) try {
        auto defaultParams = InsertArgs(argc, argv);
        if (std::get<bool>(Options["help"])) {
            std::cerr << Help << std::flush;
            std::exit(0);
        }
        if (!Default && !defaultParams.empty()) {
            throw TException{"More than ", Parameters.size(), " params"};
        }
        for (auto&& param : defaultParams) {
            Parameters.push_back(*Default);
            SetValue(Parameters.back(), param);
        }
        for (auto&& [key, value] : Options) {
            if (!std::visit(RequiredChecker, value)) {
                throw TException{"Required option ", std::quoted(key)};
            }
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n' << Help << std::flush;
        std::exit(1);
    } catch (...) {
        std::cerr << Help << std::flush;
        std::exit(1);
    }

    template <typename T, typename TCheck = TEmptyEnableIf<NInternal::IsOptionType<T>>>
    [[nodiscard]]
    const T& Get(std::string_view opt) const {
        return std::get<T>(Options.at(opt));
    }

    template <typename T, typename TCheck = TEmptyEnableIf<NInternal::IsOptionType<T>>>
    [[nodiscard]]
    const T& Get(std::size_t opt) const {
        return std::get<T>(Parameters.at(opt));
    }

    [[nodiscard]]
    std::size_t Size() const;

    [[nodiscard]]
    std::string_view GetName() const;

    [[nodiscard]]
    std::string_view GetDescription() const;

private:
    std::vector<std::string_view> InsertArgs(std::size_t argc, char* argv[]);

    template <typename... TParams, std::size_t... Indexes>
    void ParamsInit(std::index_sequence<Indexes...>) {
        (Parameters[Indexes].template emplace<TParams>(), ...);
    }

    static constexpr auto RequiredChecker = [](auto&& var) {
        using TType = std::decay_t<decltype(var)>;
        if constexpr (std::is_same_v<TType, std::string_view>) {
            return !var.empty();
        }
        return true;
    };

    static void SetValue(TCommand::TElement& elem, std::string_view str);

    template <typename... TOpts, typename... TParams, typename TParam>
    static std::string ConfigureHelp(
        std::string_view name, TParamList<TParams...> prms, TDefaultParam<TParam> prm, const TOpt<TOpts>&... opts)
    {
        std::ostringstream stream;
        stream << std::boolalpha << "Usage\n";
        stream << '\t' << name;
        if (sizeof...(opts) > 0) {
            stream << " [options]";
        }
        for (auto param : prms.Names) {
            WrapHelpParam(stream, param);
        }
        WrapHelpParam(stream, prm.Name);
        stream << "...";
        stream << '\n';
        if (sizeof...(opts) > 0) {
            stream << "Options\n";
        }
        (WrapHelpOption(stream, opts), ...);
        return stream.str();
    }

    template <typename... TOpts, typename... TParams>
    static std::string ConfigureHelp(
        std::string_view name, TParamList<TParams...> prms, const TOpt<TOpts>&... opts)
    {
        std::ostringstream stream;
        stream << std::boolalpha << "Usage\n";
        stream << '\t' << name;
        if (sizeof...(opts) > 0) {
            stream << " [options]";
        }
        for (auto param : prms.Names) {
            WrapHelpParam(stream, param);
        }
        stream << '\n';
        if (sizeof...(opts) > 0) {
            stream << "Options\n";
        }
        (WrapHelpOption(stream, opts), ...);
        return stream.str();
    }


    static void WrapHelpParam(std::ostream& stream, std::string_view str);

    template <typename TOption>
    static void WrapHelpOption(std::ostream& stream, const TOpt<TOption>& opt) {
        stream << "\t-";
        if (opt.Key.size() > 1) {
            stream << '-';
        }
        stream << opt.Key;
        PrintType<TOption>(stream);
        if (!opt.Required) {
            if constexpr (std::is_same_v<TOption, std::string_view>) {
                if (!opt.Default.empty()) {
                    stream << " = " << std::quoted(opt.Default);
                }
            } else {
                stream << " = " << opt.Default;
            }
        }
        stream << '\t';
        stream << "\n\t\t" << opt.Description << '\n';
        if (!opt.Description.empty()) {
            stream << '\n';
        }
    }

    template <typename TType>
    static void PrintType(std::ostream& stream) {
        stream << ' ' << '<';
        if (std::is_same_v<TType, std::string_view>) {
            stream << "string";
        } else if (std::is_same_v<TType, long long>) {
            stream << "int";
        } else if (std::is_same_v<TType, long double>) {
            stream << "float";
        } else {
            stream << "bool";
        }
        stream << '>';
    }

private:
    std::unordered_map<std::string_view, TElement> Options;
    std::vector<TElement> Parameters;
    std::optional<TElement> Default;
    std::string Help;
    std::string_view Name;
    std::string_view Description;
};
