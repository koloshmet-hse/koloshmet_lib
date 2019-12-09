#include "tree_value.h"

TTreeValue& TTreeValue::operator=(bool b) {
    Value = std::make_unique<TVariant>(b);
    return *this;
}

TTreeValue& TTreeValue::operator=(long long n) {
    Value = std::make_unique<TVariant>(n);
    return *this;
}

TTreeValue& TTreeValue::operator=(double d) {
    Value = std::make_unique<TVariant>(d);
    return *this;
}

TTreeValue& TTreeValue::operator=(std::string str) {
    Value = std::make_unique<TVariant>(std::move(str));
    return *this;
}

TTreeValue& TTreeValue::operator=(const TTreeValue& json) {
    if (json.Value) {
        Value = std::make_unique<TVariant>(*json.Value);
    } else {
        Value = nullptr;
    }

    return *this;
}

TTreeValue& TTreeValue::operator=(TTreeValue&& json) noexcept {
    Value = std::move(json.Value);
    return *this;
}

const TTreeValue& TTreeValue::operator[](const std::string& sv) const {
    return std::get<TDict>(*Value).at(sv);
}

TTreeValue& TTreeValue::operator[](const std::string& sv) {
    if (!Value) {
        Value = std::make_unique<TVariant>();
        Value->emplace<TDict>();
    }
    return std::get<TDict>(*Value)[sv];
}

const TTreeValue& TTreeValue::operator[](size_t index) const {
    return std::get<TArray>(*Value).at(index);
}

TTreeValue& TTreeValue::operator[](size_t index) {
    return std::get<TArray>(*Value).at(index);
}

void TTreeValue::Push(const TTreeValue& json) {
    if (!Value) {
        Value = std::make_unique<TVariant>();
        Value->emplace<TArray>();
    }
    std::get<TArray>(*Value).push_back(json);
}

void TTreeValue::Push(TTreeValue&& json) {
    if (!Value) {
        Value = std::make_unique<TVariant>();
        Value->emplace<TArray>();
    }
    std::get<TArray>(*Value).push_back(std::move(json));
}

bool TTreeValue::Contains(const std::string& key) const {
    const auto& dict = AsDict();
    return dict.find(key) != dict.end();
}

bool TTreeValue::Contains(size_t index) const {
    return AsArray().size() > index;
}
