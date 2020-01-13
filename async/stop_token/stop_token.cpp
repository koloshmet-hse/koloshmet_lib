#include "stop_token.h"

TStopToken::TStopToken()
    : Stopped{false}
{}

TStopToken::operator bool() const {
    return Stopped.load();
}

void TStopToken::Stop() {
    Stopped.store(true);
}
