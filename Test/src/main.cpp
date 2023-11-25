#define PURRNET_MAXBUF 4096
#include <PurrfectNetworking.hpp>

#include <sstream>

int main(int argc, char** argv) {
    if (!PurrfectNetworking::Initialize()) return 1;

    PurrfectNetworking::WinSocket* sock = nullptr;

    try {
        sock = new PurrfectNetworking::WinSocket();

        std::string host = "127.0.0.1";
        sock->Connect(host, 3000);

        std::string path = "/";

        std::ostringstream requestStream;
        requestStream << "GET " << path << " HTTP/1.1\r\n";
        requestStream << "Host: " << host << "\r\n";
        requestStream << "Connection: close\r\n\r\n";

        sock->Send((char*) requestStream.str().c_str());
        auto data = sock->Recieve();

        std::cout << data.Data << std::endl;

    } catch (std::exception ex) {
        PURRNET_LOG_ERR(ex.what());
        goto cleanup;
    }

cleanup:
    if (sock) delete sock;

    PurrfectNetworking::Shutdown();

    return 0;
}