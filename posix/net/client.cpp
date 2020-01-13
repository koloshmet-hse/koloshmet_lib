#include "client.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <netdb.h>

TConnectedSocket NInternal::ConnectIP(const std::string& address, int port) {
    TSocket socket{ESocket::IP};
    auto& sockAddr = socket.Address<sockaddr_in>();
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = htons(port);
    if (auto host = gethostbyname(address.c_str()); host != nullptr) {
        std::memcpy(std::addressof(sockAddr.sin_addr), host->h_addr_list[0], sizeof(sockAddr.sin_addr));
    }

    return Connect(std::move(socket));
}

TConnectedSocket NInternal::ConnectUNIX(const std::filesystem::path& socketPath) {
    TSocket socket{ESocket::UNIX};
    auto& sockAddr = socket.Address<sockaddr_un>();
    sockAddr.sun_family = AF_LOCAL;
    std::strncpy(sockAddr.sun_path, socketPath.c_str(), sizeof(sockAddr.sun_path) - 1);

    return Connect(std::move(socket));
}
