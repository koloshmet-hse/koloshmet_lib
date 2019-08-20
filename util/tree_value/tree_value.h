#pragma once

#include <deque>
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

    TTreeValue& operator=(bool b) {
        Value = std::make_unique<TVariant>(b);
        return *this;
    }

    TTreeValue& operator=(long long n) {
        Value = std::make_unique<TVariant>(n);
        return *this;
    }

    TTreeValue& operator=(double d) {
        Value = std::make_unique<TVariant>(d);
        return *this;
    }

    TTreeValue& operator=(std::string_view str) {
        Value = std::make_unique<TVariant>(static_cast<std::string>(str));
        return *this;
    }

    TTreeValue& operator=(const TTreeValue& json) {
        if (json.Value) {
            Value = std::make_unique<TVariant>(*json.Value);
        } else {
            Value = nullptr;
        }

        return *this;
    }

    TTreeValue& operator=(TTreeValue&& json) noexcept {
        Value = std::move(json.Value);
        return *this;
    }

    const TTreeValue& operator[](std::string_view sv) const {
        return std::get<TDict>(*Value).at(static_cast<std::string>(sv));
    }

    TTreeValue& operator[](std::string_view sv) {
        if (!Value) {
            Value = std::make_unique<TVariant>();
            Value->emplace<TDict>();
        }
        return std::get<TDict>(*Value)[static_cast<std::string>(sv)];
    }

    const TTreeValue& operator[](size_t index) const {
        return std::get<TArray>(*Value).at(index);
    }

    TTreeValue& operator[](size_t index) {
        return std::get<TArray>(*Value).at(index);
    }

    void Push(const TTreeValue& json) {
        if (!Value) {
            Value = std::make_unique<TVariant>();
            Value->emplace<TArray>();
        }
        std::get<TArray>(*Value).push_back(json);
    }

    void Push(TTreeValue&& json) {
        if (!Value) {
            Value = std::make_unique<TVariant>();
            Value->emplace<TArray>();
        }
        std::get<TArray>(*Value).push_back(std::move(json));
    }

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

    const TDict& AsDict() const {
        return std::get<TDict>(*Value);
    }

    TDict& AsDictMutable() {
        return std::get<TDict>(*Value);
    }

    const TArray& AsArray() const {
        return std::get<TArray>(*Value);
    }

    TArray& AsArrayMutable() {
        return std::get<TArray>(*Value);
    }

    bool Undefined() const {
        return !static_cast<bool>(Value);
    }

private:
    std::unique_ptr<TVariant> Value;

private:
    friend class TJsonIO;
};
