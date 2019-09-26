#pragma once

#include "unique_fd.h"

#include <istream>
#include <ostream>
#include <streambuf>

#include <memory>

namespace NInternal {
    size_t BuffSize();

    size_t Read(const IFd& fd, std::byte* data, size_t sz);

    size_t Write(const IFd& fd, const std::byte* data, size_t sz);
}

template <typename TChar, typename TCloser>
class TBasicIFdStreamBuf : public std::basic_streambuf<TChar> {
public:
    TBasicIFdStreamBuf(TBasicUniqueFd<TCloser>&& fd, size_t buffSize)
        : Buffer(std::make_unique<TChar[]>(buffSize))
        , Size{buffSize}
        , Fd{std::move(fd)}
    {
        auto start = Buffer.get();
        auto end = start + Size;
        this->setg(start, end, end);
    }

    TBasicIFdStreamBuf(TBasicIFdStreamBuf&& streamBuf) noexcept
        : Buffer{std::move(streamBuf.Buffer)}
        , Size{streamBuf.Size}
        , Fd{std::move(streamBuf.Fd)}
    {
        this->setg(Buffer.get(), streamBuf.gptr(), Buffer.get() + Size);
    }

    TBasicIFdStreamBuf& operator=(TBasicIFdStreamBuf&& streamBuf) noexcept {
        Buffer = std::move(streamBuf.Buffer);
        Size = streamBuf.Size;
        Fd = std::move(streamBuf.Fd);
        auto start = Buffer.get();
        auto end = start + Size;
        this->setg(start, start + std::distance(streamBuf.eback(), streamBuf.gptr()), end);
        return *this;
    }

    int underflow() override {
        if (this->gptr() < this->egptr()) {
            return *this->gptr();
        }

        if (sync() == 0) {
            return *this->gptr();
        }
        return std::basic_streambuf<TChar>::traits_type::eof();
    }

    int sync() override {
        auto start = this->eback();
        auto rd = NInternal::Read(Fd, reinterpret_cast<std::byte*>(start), Size * sizeof(TChar));
        this->setg(start, start, start + rd);
        if (rd > 0) {
            return 0;
        }
        return -1;
    }

    void Open(TBasicUniqueFd<TCloser>&& fd) {
        Fd.Reset();
        Fd = std::move(fd);
    }

    [[nodiscard]]
    bool IsOpen() const {
        return static_cast<bool>(Fd);
    }

    void Close() {
        Fd.Reset();
    }

private:
    std::unique_ptr<TChar[]> Buffer;
    size_t Size;
    TBasicUniqueFd<TCloser> Fd;
};

template <typename TChar, typename TCloser>
class TBasicOFdStreamBuf : public std::basic_streambuf<TChar> {
public:
    TBasicOFdStreamBuf(TBasicUniqueFd<TCloser>&& fd, size_t buffSize)
        : Buffer(std::make_unique<TChar[]>(buffSize))
        , Size(buffSize)
        , Fd{std::move(fd)}
    {
        auto start = Buffer.get();
        auto end = start + Size - 1;
        this->setp(start, end);
    }

    TBasicOFdStreamBuf(TBasicOFdStreamBuf&& streamBuf) noexcept
        : Buffer{std::move(streamBuf.Buffer)}
        , Size{streamBuf.Size}
        , Fd{std::move(streamBuf.Fd)}
    {
        auto start = Buffer.get();
        auto end = start + Size - 1;
        this->setp(start, end);
        this->pbump(streamBuf.pptr() - streamBuf.pbase());
    }

    TBasicOFdStreamBuf& operator=(TBasicOFdStreamBuf&& streamBuf) noexcept {
        Buffer = std::move(streamBuf.Buffer);
        Size = streamBuf.Size;
        Fd = std::move(streamBuf.Fd);
        auto start = Buffer.get();
        auto end = start + Size - 1;
        this->setp(start, end);
        this->pbump(streamBuf.pptr() - streamBuf.pbase());
        return *this;
    }

    int overflow(int c) override {
        if (c != std::basic_streambuf<TChar>::traits_type::eof()) {
            *this->pptr() = c;
            this->pbump(1);

            if (sync() == 0) {
                return c;
            }
        }

        return std::basic_streambuf<TChar>::traits_type::eof();
    }

    int sync() override {
        auto sz = this->pptr() - this->pbase();
        if (sz > 0) {
            auto wr = NInternal::Write(Fd, reinterpret_cast<std::byte*>(this->pbase()), sz * sizeof(TChar));
            if (wr > 0) {
                this->pbump(-wr);
                return 0;
            }
            return -1;
        }
        return 0;
    }

    void Open(TBasicUniqueFd<TCloser>&& fd) {
        Fd.Reset();
        Fd = std::move(fd);
    }

    [[nodiscard]]
    bool IsOpen() const {
        return static_cast<bool>(Fd);
    }

    void Close() {
        Fd.Reset();
    }

private:
    std::unique_ptr<TChar[]> Buffer;
    size_t Size;
    TBasicUniqueFd<TCloser> Fd;
};

template <typename TChar, typename TCloser>
class TBasicFdStreamBuf : public std::basic_streambuf<TChar> {
public:
    explicit TBasicFdStreamBuf(TBasicUniqueFd<TCloser>&& fd, size_t buffSize)
        : Buffer(std::make_unique<TChar[]>(buffSize))
        , Size(buffSize)
        , Fd{std::move(fd)}
    {
        auto start = Buffer.get();
        auto end = start + Size;
        this->setg(start, end, end);
        this->setp(start, end - 1);
    }

    TBasicFdStreamBuf(TBasicFdStreamBuf&& streamBuf) noexcept
        : Buffer{std::move(streamBuf.Buffer)}
        , Size{streamBuf.Size}
        , Fd{std::move(streamBuf.Fd)}
    {
        auto start = Buffer.get();
        auto end = start + Size;
        this->setg(start, start + std::distance(streamBuf.eback(), streamBuf.gptr()), end);
        this->setp(start, end - 1);
        this->pbump(streamBuf.pptr() - streamBuf.pbase());
    }

    TBasicFdStreamBuf& operator=(TBasicFdStreamBuf&& streamBuf) noexcept {
        Buffer = std::move(streamBuf.Buffer);
        Size = streamBuf.Size;
        Fd = std::move(streamBuf.Fd);
        auto start = Buffer.get();
        auto end = start + Size;
        this->setg(start, start + streamBuf.gptr() - streamBuf.eback(), end);
        this->setp(start, end - 1);
        this->pbump(streamBuf.pptr() - streamBuf.pbase());
        return *this;
    }

    int underflow() override {
        if (this->gptr() < this->egptr()) {
            return *this->gptr();
        }

        auto start = this->eback();
        auto rd = NInternal::Read(Fd, reinterpret_cast<std::byte*>(start), Size * sizeof(TChar));
        this->setg(start, start, start + rd);

        if (rd > 0) {
            return *this->gptr();
        }
        return std::basic_streambuf<TChar>::traits_type::eof();
    }

    int overflow(int c) override {
        if (c != std::basic_streambuf<TChar>::traits_type::eof()) {
            *this->pptr() = c;
            this->pbump(1);

            if (sync() == 0) {
                return c;
            }
        }

        return std::basic_streambuf<TChar>::traits_type::eof();
    }

    int sync() override {
        auto sz = this->pptr() - this->pbase();
        if (sz > 0) {
            auto wr = NInternal::Write(Fd, reinterpret_cast<std::byte*>(this->pbase()), sz * sizeof(TChar));
            if (wr > 0) {
                this->pbump(-wr);
                return 0;
            }
            return -1;
        }
        return 0;
    }

    void Open(TBasicUniqueFd<TCloser>&& fd) {
        Fd.Reset();
        Fd = std::move(fd);
    }

    [[nodiscard]]
    bool IsOpen() const {
        return static_cast<bool>(Fd);
    }

    void Close() {
        Fd.Reset();
    }

private:
    std::unique_ptr<TChar[]> Buffer;
    size_t Size;
    TBasicUniqueFd<TCloser> Fd;
};

template <typename TChar, typename TCloser = TFdCloser>
class TBasicIFdStream : public std::basic_istream<TChar> {
public:
    explicit TBasicIFdStream(TBasicUniqueFd<TCloser>&& fd)
        : std::basic_istream<TChar>{nullptr}
        , StreamBuf{std::move(fd), NInternal::BuffSize()}
    {
        this->rdbuf(std::addressof(StreamBuf));
    }

    TBasicIFdStream(TBasicIFdStream&& stream) noexcept
        : std::basic_istream<TChar>{nullptr}
        , StreamBuf{std::move(stream.StreamBuf)}
    {
        this->rdbuf(std::addressof(StreamBuf));
    }

    TBasicIFdStream& operator=(TBasicIFdStream&& stream) noexcept {
        StreamBuf = std::move(stream.StreamBuf);
        this->rdbuf(std::addressof(StreamBuf));
        return *this;
    }

    void Open(TBasicUniqueFd<TCloser>&& fd) {
        StreamBuf.Open(std::move(fd));
    }

    [[nodiscard]]
    bool IsOpen() const {
        return StreamBuf.IsOpen();
    }

    void Close() {
        StreamBuf.Close();
    }

private:
    TBasicIFdStreamBuf<TChar, TCloser> StreamBuf;
};

template <typename TChar, typename TCloser = TFdCloser>
class TBasicOFdStream : public std::basic_ostream<TChar> {
public:
    explicit TBasicOFdStream(TBasicUniqueFd<TCloser>&& fd)
        : std::basic_ostream<TChar>{nullptr}
        , StreamBuf{std::move(fd), NInternal::BuffSize()}
    {
        this->rdbuf(std::addressof(StreamBuf));
    }

    TBasicOFdStream(TBasicOFdStream&& stream) noexcept
        : std::basic_ostream<TChar>{nullptr}
        , StreamBuf{std::move(stream.StreamBuf)}
    {
        this->rdbuf(std::addressof(StreamBuf));
    }

    TBasicOFdStream& operator=(TBasicOFdStream&& stream) noexcept {
        StreamBuf = std::move(stream.StreamBuf);
        this->rdbuf(std::addressof(StreamBuf));
        return *this;
    }

    ~TBasicOFdStream() override {
        this->flush();
    }

    void Open(TBasicUniqueFd<TCloser>&& fd) {
        this->flush();
        StreamBuf.Open(std::move(fd));
    }

    [[nodiscard]]
    bool IsOpen() const {
        return StreamBuf.IsOpen();
    }

    void Close() {
        this->flush();
        StreamBuf.Close();
    }

private:
    TBasicOFdStreamBuf<TChar, TCloser> StreamBuf;
};

template <typename TChar, typename TCloser = TFdCloser>
class TBasicFdStream : public std::basic_iostream<TChar> {
public:
    explicit TBasicFdStream(TBasicUniqueFd<TCloser>&& fd)
        : std::basic_iostream<TChar>{nullptr}
        , StreamBuf{std::move(fd), NInternal::BuffSize()}
    {
        this->rdbuf(std::addressof(StreamBuf));
    }

    TBasicFdStream(TBasicFdStream&& stream) noexcept
        : std::basic_iostream<TChar>{nullptr}
        , StreamBuf{std::move(stream.StreamBuf)}
    {
        this->rdbuf(std::addressof(StreamBuf));
    }

    TBasicFdStream& operator=(TBasicFdStream&& stream) noexcept {
        StreamBuf = std::move(stream.StreamBuf);
        this->rdbuf(std::addressof(StreamBuf));
        return *this;
    }

    ~TBasicFdStream() override {
        this->flush();
    }

    void Open(TBasicUniqueFd<TCloser>&& fd) {
        this->flush();
        StreamBuf.Open(std::move(fd));
    }

    [[nodiscard]]
    bool IsOpen() const {
        return StreamBuf.IsOpen();
    }

    void Close() {
        this->flush();
        StreamBuf.Close();
    }

private:
    TBasicFdStreamBuf<TChar, TCloser> StreamBuf;
};

using TIFdStream = TBasicIFdStream<char>;
using TOFdStream = TBasicOFdStream<char>;
using TFdStream = TBasicFdStream<char>;
