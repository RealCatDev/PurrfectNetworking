#define PURRNET_USE_DP
#define PURRNET_DP 6969
#define PURRNET_MAXBUF 4096
#include <PurrfectNetworking.hpp>

#include <sstream>

struct User {
    PURRNET_NS::Socket* socket;
    std::string name, ip;
    int id, roomId;
};

struct Room{
    int ID;

    Room() = default;

    Room(int id)
        : ID(id) {
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
            auto user = CreateUser(socket);
            auto idStr = std::to_string(user.id);
            const char *id = idStr.c_str();
            socket->Send("meow");
            std::cout << id << ":" << strlen(id) << std::endl;
            PURRNET_LOG_INF(PURRNET_FMT("[%s] Connected!", msg.c_str()));
        });

        on("onMessage", [this](PURRNET_NS::Socket* socket, std::string data) {
            std::string ip, msg;
            {
                size_t pipe = data.find('|');
                ip = data;
                ip = ip.erase(pipe);
                msg = data;
                msg = msg.erase(0, pipe+1);
            }
            auto user = GetUserByIP(ip);
            if (user.name.empty()) {
                user.name = msg;
            } else {
                if (msg.starts_with(":join")) {
                    size_t space = msg.find(' ');
                    std::string room = msg;
                    room.erase(space);
                    uint32_t roomId = static_cast<uint32_t>(atoi(room.c_str()));
                } else if (msg == ":createRoom") {
                    auto room = CreateRoom();
                    socket->emit("joinRoom", socket, std::to_string(room.ID));
                } else MessageAll(user.name + ": " + msg, socket);
            }
        });

        on("onDisconnected", [this](PURRNET_NS::Socket* socket, std::string data) {
            PURRNET_LOG_INF(PURRNET_FMT("[%s] Disconnected!", data.c_str()));
        });
    }

private:

    User &CreateUser(PURRNET_NS::Socket* sock) {
        int id = m_IpToID.size();
        m_IpToID[sock->GetIpAddress()] = id;
        auto usr = User();
        usr.id = id;
        usr.ip = sock->GetIpAddress();
        usr.socket = sock;
        if (m_Users.size() <= id) m_Users.resize(id+1);
        m_Users[id] = usr;
        return m_Users[id];
    }

    User &GetUserByIP(std::string ip) {
        auto id = m_IpToID[ip];
        return m_Users[id];
    }

    Room& CreateRoom() {
        Room room{};
        int id = m_Rooms.size();
        room.ID = id;
        if (m_Rooms.size() < id) m_Users.resize(id);
        m_Rooms[id] = room;
        return m_Rooms[id];
    }

    std::unordered_map<std::string, int> m_IpToID{};
    std::vector<User> m_Users{};
    std::vector<Room> m_Rooms{};

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