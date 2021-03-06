#pragma once

#include <istream>
#include <ostream>

template <typename T>
class TBinaryIO {
    static_assert(std::is_trivial_v<T> && std::is_standard_layout_v<T>);

public:
    explicit TBinaryIO(T& value)
        : Value{value}
    {}

public:
    T& Value;
};

template <typename T>
std::istream& operator>>(std::istream& stream, TBinaryIO<T>& value) {
    stream.read(reinterpret_cast<char*>(std::addressof(value.Value)), sizeof(T));
    return stream;
}

template <typename T>
std::istream& operator>>(std::istream& stream, TBinaryIO<T>&& value) {
    stream.read(reinterpret_cast<char*>(std::addressof(value.Value)), sizeof(T));
    return stream;
}

template <typename T>
std::ostream& operator<<(std::ostream& stream, const TBinaryIO<T>& value) {
    stream.write(reinterpret_cast<const char*>(std::addressof(value.Value)), sizeof(T));
    return stream;
}
