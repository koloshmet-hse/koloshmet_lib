#pragma once

#include <atomic>

class TStopToken {
public:
    TStopToken();

    explicit operator bool() const;

    void Stop();
private:
    std::atomic_bool Stopped;
};
