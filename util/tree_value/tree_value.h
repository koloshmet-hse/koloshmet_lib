#pragma once

#include <deque>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>

class TTreeValue {
    using TArray = std::deque<TTreeValue>;
    using TDict = std::unordered_map<std::string, TTreeValue>;
    using TString = std::string;
    using TVariant = std::variant<bool, long long, double, TString, TArray, TDict>;

public:
    TTreeValue()
        : Value(nullptr)
    {}

    TTreeValue(bool b)
        : Value(std::make_unique<TVariant>(b))
    {}

    TTreeValue(long long v)
        : Value(std::make_unique<TVariant>(v))
    {}

    TTreeValue(double d)
        : Value(std::make_unique<TVariant>(d))
    {}

    TTreeValue(std::string_view str)
        : Value(std::make_unique<TVariant>(static_cast<std::string>(str)))
    {}

    TTreeValue(const TTreeValue& json)
        : Value(std::make_unique<TVariant>(*json.Value))
    {}

    TTreeValue(TTreeValue&& json) noexcept
        : Value(std::move(json.Value))
    {}

    TTreeValue& operator=(bool b);

    TTreeValue& operator=(long long n);

    TTreeValue& operator=(double d);

    TTreeValue& operator=(std::string_view str);

    TTreeValue& operator=(const TTreeValue& json);

    TTreeValue& operator=(TTreeValue&& json) noexcept;

    const TTreeValue& operator[](std::string_view sv) const;

    TTreeValue& operator[](std::string_view sv);

    const TTreeValue& operator[](size_t index) const;

    TTreeValue& operator[](size_t index);

    void Push(const TTreeValue& json);

    void Push(TTreeValue&& json);

    [[nodiscard]]
    bool Contains(std::string_view key) const;

    [[nodiscard]]
    bool Contains(size_t index) const;

    explicit operator bool() const {
        return std::get<bool>(*Value);
    }

    explicit operator long long() const {
        return std::get<long long>(*Value);
    }

    explicit operator double() const {
        return std::get<double>(*Value);
    }

    explicit operator std::string() const {
        return std::get<TString>(*Value);
    }

    [[nodiscard]]
    const TDict& AsDict() const {
        return std::get<TDict>(*Value);
    }

    TDict& AsDictMutable() {
        return std::get<TDict>(*Value);
    }

    [[nodiscard]]
    const TArray& AsArray() const {
        return std::get<TArray>(*Value);
    }

    TArray& AsArrayMutable() {
        return std::get<TArray>(*Value);
    }

    [[nodiscard]]
    bool Undefined() const {
        return !static_cast<bool>(Value);
    }

private:
    std::unique_ptr<TVariant> Value;

private:
    friend class TJsonIO;
};
