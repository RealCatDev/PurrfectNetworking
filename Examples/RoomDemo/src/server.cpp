#define PURRNET_USE_DP
#define PURRNET_DP 6969
#define PURRNET_MAXBUF 4096
#include <PurrfectNetworking.hpp>

#include <sstream>

struct User {
    std::string name, ip;
    PURRNET_NS::Socket* socket;
    uint32_t ID, roomId;

    User() = default;

    User(uint32_t id, std::string ip, PURRNET_NS::Socket* sock)
        : name(""), ip(ip), socket(sock), ID(id), roomId(-1) {
    }
};

struct Room{
    uint32_t ID;
    uint32_t OwnerID;

    Room() = default;

    Room(uint32_t id, User owner)
        : ID(id), OwnerID(owner.ID) {
    }
};

class Store {

public:

    Store() = default;
    ~Store() = default;

    inline User& RegisterUser(PURRNET_NS::Socket *sock) {
        m_Users[m_Users.size()] = User(m_Users.size(), sock->GetIpAddress(), sock);
        return m_Users[m_Users.size() - 1];
    }

    inline Room& RegisterRoom(User owner) {
        m_Rooms[m_Rooms.size()] = Room(m_Rooms.size(), owner);
        return m_Rooms[m_Rooms.size() - 1];
    }

    inline User &GetUserByIP(std::string ip) {
        auto it = std::find(m_Users.begin(), m_Users.end(), [ip](User* u) { return u->ip == ip; });
        if (it == m_Users.end()) return (User&)User();
        return (*it).second;
    }

    inline std::unordered_map<uint32_t, User>::iterator GetUserItByIP(std::string ip) {
        auto it = std::find(m_Users.begin(), m_Users.end(), [ip](User* u) { return u->ip == ip; });
        if (it == m_Users.end()) return {};
        return it;
    }

    inline void RemoveIP(std::string ip) {
        m_Users.erase(GetUserItByIP(ip));
    }

    inline Room& GetRoom(uint32_t id) {
        return m_Rooms[id];
    }

    inline std::unordered_map<uint32_t, Room>::iterator GetRoomIt(uint32_t id) {
        return std::find(m_Rooms.begin(), m_Rooms.end(), [id](Room* r) { return id == r->ID; });
    }

    inline Room& GetRoomByOwner(User owner) {
        auto it = std::find(m_Rooms.begin(), m_Rooms.end(), [owner](Room* r) { return r->OwnerID == owner.ID; });
        if (it == m_Rooms.end()) return (Room&)Room();
        return (*it).second;
    }

    inline std::unordered_map<uint32_t, Room>::iterator GetRoomItByOwner(User owner) {
        return std::find(m_Rooms.begin(), m_Rooms.end(), [owner](Room* r) { return owner.ID == r->OwnerID; });
    }

    inline void RemoveRoom(uint32_t id) {
        m_Rooms.erase(GetRoomIt(id));
    }

    inline void RemoveRoomByOwner(User owner) {
        m_Rooms.erase(GetRoomItByOwner(owner));
    }

private:

    std::unordered_map<uint32_t, User> m_Users;
    std::unordered_map<uint32_t, Room> m_Rooms;

};

class ExampleServer : public PURRNET_NS::Server {

public:

    ExampleServer() 
        : PURRNET_NS::Server(), m_Store(std::make_unique<Store>()) {
    }

    ~ExampleServer() {

    }

    void InitializeEvents() {
        on("onConnected", [this](PURRNET_NS::Socket *socket, std::string msg) {
            socket->Send(std::to_string(m_Store->RegisterUser(socket).ID).c_str());
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
            auto user = m_Store->GetUserByIP(ip);
            if (user.name.empty()) {
                user.name = msg;
            } else {
                if (msg.starts_with(":join")) {
                    size_t space = msg.find(' ');
                    std::string room = msg;
                    room.erase(space);
                    uint32_t roomId = static_cast<uint32_t>(atoi(room.c_str()));
                } else if (msg == ":createRoom") {
                    auto room = m_Store->RegisterRoom(user);
                    socket->emit("joinRoom", std::to_string(room.ID));
                } else MessageAll(user.name + ": " + msg, socket);
            }
        });

        on("onDisconnected", [this](PURRNET_NS::Socket* socket, std::string data) {
            PURRNET_LOG_INF(PURRNET_FMT("[%s] Disconnected!", data.c_str()));

            m_Store->RemoveIP(data);
        });
    }

private:

    std::unique_ptr<Store> m_Store;
    std::unordered_map<std::string, int> m_IpToID{};

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