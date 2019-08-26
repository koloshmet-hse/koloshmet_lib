#pragma once

#include "unique_fd.h"

#include <istream>
#include <ostream>
#include <streambuf>

#include <memory>

namespace NInternal {
    size_t BuffSize();

    size_t Read(const IFd& fd, void* data, size_t sz);

    size_t Write(const IFd& fd, const void* data, size_t sz);
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
        auto rd = NInternal::Read(Fd, start, Size * sizeof(TChar));
        this->setg(start, start, start + rd);
        if (rd > 0) {
            return 0;
        }
        return -1;
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
        auto wr = NInternal::Write(Fd, this->pbase(), sz * sizeof(TChar));
        if (wr > 0) {
            this->pbump(-wr);
            return 0;
        }
        return -1;
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

    int underflow() override {
        if (this->gptr() < this->egptr()) {
            return *this->gptr();
        }

        auto start = this->eback();
        auto rd = NInternal::Read(Fd, start, Size * sizeof(TChar));
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
        auto wr = NInternal::Write(Fd, this->pbase(), sz * sizeof(TChar));
        if (wr > 0) {
            this->pbump(-wr);
            return 0;
        }
        return -1;
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

    ~TBasicOFdStream() override {
        this->flush();
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

    ~TBasicFdStream() override {
        this->flush();
    }

private:
    TBasicFdStreamBuf<TChar, TCloser> StreamBuf;
};

using TIFdStream = TBasicIFdStream<char>;
using TOFdStream = TBasicOFdStream<char>;
using TFdStream = TBasicFdStream<char>;
