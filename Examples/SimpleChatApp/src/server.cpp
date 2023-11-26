#define PURRNET_USE_DP
#define PURRNET_DP 6969
#define PURRNET_MAXBUF 4096
#include <PurrfectNetworking.hpp>

#include <sstream>

struct User {

    std::string name;
    PURRNET_NS::Socket *socket;

};

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
        int id = m_IdCounter++;

        PURRNET_LOG_INF(PURRNET_FMT("Client connected! Assigned id: %llu.", id));
        sock->Send((std::to_string(id)).data());

        {
            PURRNET_NS::RecieveData data;
            try {
                data = sock->Recieve();
            } catch (PURRNET_NS::ClientDisconnectedException ex) {
                PURRNET_LOG_INF(PURRNET_FMT("Client disconnected! ID: %llu.", id));
                return;
            } catch (std::exception ex) {
                PURRNET_LOG_ERR(ex.what());
                Stop();
            }
            User user{};
            user.name = data.buffer;
            user.socket = sock;
            m_Users[id] = user;
        }

        while (m_Running) {
            try {
                auto data = sock->Recieve();
                if (data.size <= 0 && std::string(data.buffer) == "disconnect") {
                    PURRNET_LOG_INF(PURRNET_FMT("Client disconnected! ID: %llu.", id));
                    m_Users.erase(id);
                    delete sock;
                    return;
                } else {
                    std::ostringstream message{};
                    message << m_Users[id].name;
                    message << ": ";
                    message << std::string(data.buffer);
                    PURRNET_LOG_INF(message.str());
                    MessageAll(message.str());
                }
            } catch (std::exception ex) {
                PURRNET_LOG_ERR(ex.what());
                m_Running = false;
            }
        }
    }

    uint32_t m_IdCounter = 1;
    std::unordered_map<int, User> m_Users{};

};

int main(int argc, char** argv) {
    if (!PurrfectNetworking::Initialize()) return 1;

    ExampleServer* server = nullptr;
    std::string input = "";

    server = new ExampleServer();
    server->Run();
    while (server->Running()) {
        try {
        } catch (std::exception ex) {
            PURRNET_LOG_ERR(ex.what());
            server->Stop();
        }
    }

cleanup:
    if (server) delete server;

    PurrfectNetworking::Shutdown();

    return 0;
}