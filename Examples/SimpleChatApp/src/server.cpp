#define PURRNET_USE_DP
#define PURRNET_DP 6969
#define PURRNET_MAXBUF 4096
#include <PurrfectNetworking.hpp>

#include <sstream>

struct User {

    std::string name;
    PURRNET_NS::Socket *socket;

    User(std::string n, PURRNET_NS::Socket* sock)
        : name(n), socket(sock) {
    }

};

class ExampleServer : public PURRNET_NS::Server {

public:

    ExampleServer() 
        : PURRNET_NS::Server() {
    }

    ~ExampleServer() {

    }

    void InitializeEvents() {
        on("onConnected", [this](PURRNET_NS::Socket *socket, std::string msg) {
            m_IpToID[msg] = m_IdCounter;
            socket->Send(std::to_string(m_IdCounter++).c_str());
            PURRNET_LOG_INF(PURRNET_FMT("[%s] Connected!", msg.c_str()));
        });

        on("onMessage", [this](PURRNET_NS::Socket* socket, std::string data) {
            size_t pipe = data.find('|');
            std::string ip = data;
            ip = ip.erase(pipe);
            std::string msg = data;
            msg = msg.erase(0, pipe+1);
            auto user = m_Users[m_IpToID[ip]];
            if (!user) {
                user = m_Users[m_IpToID[ip]] = new User(msg, socket);
                MessageAll("[" + user->name + " has joined the chat.]");
            } else MessageAll(user->name + ": " + msg, socket);
        });

        on("onDisconnected", [this](PURRNET_NS::Socket* socket, std::string data) {
            PURRNET_LOG_INF(PURRNET_FMT("[%s] Disconnected!", data.c_str()));
            if (m_Users.find(m_IpToID[data]) != m_Users.end()) MessageAll("[\"" + m_Users[m_IpToID[data]]->name + "\" has disconnected.]", socket);
            if (m_Users.find(m_IpToID[data]) != m_Users.end()) m_Users.erase(m_IpToID[data]);
            m_IpToID.erase(data);
        });
    }

private:

    uint32_t m_IdCounter = 1;
    std::unordered_map<std::string, int> m_IpToID{};
    std::unordered_map<int, User*> m_Users{};

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

    if (server) delete server;

    PurrfectNetworking::Shutdown();

    return 0;
}