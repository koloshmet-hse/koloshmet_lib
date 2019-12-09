#pragma once

#include <posix/net/socket.h>

class IReplier {
public:
    virtual ~IReplier() = default;

    virtual void Connect(TConnectedSocket&& newSocket) = 0;
};
