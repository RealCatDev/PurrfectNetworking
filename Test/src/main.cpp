#define PURRNET_USE_DP
#define PURRNET_DP 6969
#define PURRNET_MAXBUF 4096
#include <PurrfectNetworking.hpp>

class ExampleServer : public PURRNET_NS::Server {

public:

    ExampleServer() 
        : PURRNET_NS::Server() {
    }

    ~ExampleServer() {

    }

private:

    virtual void ClientThread(PURRNET_NS::Socket *sock) override {
        char buf[PURRNET_MAXBUF];
        int bytesRead = 0;
        while (m_Running) {
            try {
                auto data = sock->Recieve();
                if (data.size > 0 && std::string(data.buffer) != "disconnect") continue;
                PURRNET_LOG_INF("Client disconnected!");
                delete sock;
                return;
            } catch (std::exception ex) {
                PURRNET_LOG_ERR(ex.what());
                m_Running = false;
            }
        }
    }

};

int main(int argc, char** argv) {
    if (!PurrfectNetworking::Initialize()) return 1;

    ExampleServer* server = nullptr;
    std::string input = "";

    try {
        server = new ExampleServer();
        server->Run();
        while (server->Running()) {
            std::getline(std::cin, input);
            if (input == "!quit") server->Stop();
        }
    } catch (std::exception ex) {
        PURRNET_LOG_ERR(ex.what());
        goto cleanup;
    }

cleanup:
    if (server) delete server;

    PurrfectNetworking::Shutdown();

    return 0;
}