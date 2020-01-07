#pragma once

#include <util/opt/opt.h>

#include <variant>
#include <unordered_map>
#include <vector>

#include <string_view>

#include <filesystem>

#include <util/exception/exception.h>

#include <iostream>

class TOptions {
    using TElement = std::variant<std::string_view, long double, long long, bool>;
    using TElementTypes = TTypeList<std::string_view, long double, long long, bool>;

public:
    template <typename... TOpts, typename... TParams, typename TParam>
    TOptions(
        int argc, char* argv[],
        TParamList<TParams...> params, TDefaultParam<TParam> par, const TOpt<TOpts>&... options) try
        : Options{}
        , Parameters(sizeof...(TParams) + 1)
    {
        (Options[options.Key].template emplace<TOpts>(options.Default), ...);
        ParamsInit<std::string_view, TParams...>(std::index_sequence_for<std::string_view, TParams...>{});

        auto defaultParams = InsertArgs(argc, argv);
        for (auto&& param : defaultParams) {
            Parameters.emplace_back(std::in_place_type_t<TParam>{});
            SetValue(Parameters.back(), param);
        }

        bool noRequired = false;
        (noRequired |= ... |= (options.Required && std::get<TOpts>(Options[options.Key]) == options.Default));
        if (noRequired) {
            throw TException{"No required"};
        }
    } catch (...) {
        if (argc < 1) {
            throw TException{"Irrelevant argc"};
        }
        std::cerr << ConfigureHelp(argv[0], params, par, options...) << std::endl;
        std::exit(1);
    }

    template <typename... TOpts, typename... TParams>
    TOptions(
        int argc, char* argv[],
        TParamList<TParams...> params, const TOpt<TOpts>&... options) try
        : Options{}
        , Parameters(sizeof...(TParams) + 1)
    {
        (Options[options.Key].template emplace<TOpts>(options.Default), ...);
        ParamsInit<std::string_view, TParams...>(std::index_sequence_for<std::string_view, TParams...>{});

        auto defaultParams = InsertArgs(argc, argv);
        if (!defaultParams.empty()) {
            throw TException{"More than ", sizeof...(TParams) + 1, " params"};
        }

        bool noRequired = false;
        (noRequired |= ... |= (options.Required && std::get<TOpts>(Options[options.Key]) == options.Default));
        if (noRequired) {
            throw TException{"No required"};
        }
    } catch (...) {
        if (argc < 1) {
            throw TException{"Irrelevant argc"};
        }
        std::cerr << ConfigureHelp(argv[0], params, options...) << std::endl;
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

private:
    std::vector<std::string_view> InsertArgs(std::size_t argc, char* argv[]);

    template <typename... TParams, std::size_t... Indexes>
    void ParamsInit(std::index_sequence<Indexes...>) {
        (Parameters[Indexes].template emplace<TParams>(), ...);
    }

    static void SetValue(TOptions::TElement& elem, std::string_view str);

    template <typename... TOpts, typename... TParams, typename TParam>
    static std::string ConfigureHelp(
        std::string_view name, TParamList<TParams...> prms, TDefaultParam<TParam> prm, const TOpt<TOpts>&... opts)
    {
        std::ostringstream stream;
        std::filesystem::path path{name};
        stream << std::boolalpha << "Usage\n";
        stream << '\t' << path.filename().string();
        stream << " [options]";
        for (auto param : prms.Names) {
            WrapHelpParam(stream, param);
        }
        WrapHelpParam(stream, prm.Name);
        stream << "...";
        stream << "\nOptions\n";
        (WrapHelpOption(stream, opts), ...);
        return stream.str();
    }

    template <typename... TOpts, typename... TParams>
    static std::string ConfigureHelp(
        std::string_view name, TParamList<TParams...> prms, const TOpt<TOpts>&... opts)
    {
        std::ostringstream stream;
        std::filesystem::path path{name};
        stream << std::boolalpha << "Usage\n";
        stream << '\t' << path.filename().string();
        stream << " [options]";
        for (auto param : prms.Names) {
            WrapHelpParam(stream, param);
        }
        stream << "\nOptions\n";
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
            stream << " = " << opt.Default;
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
};
