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

    bool operator ==(Room& other) {
        return ID == other.ID;
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
            socket->SendMsg(id);
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
                SetUsersName(ip, msg);
            } else {
                std::cout << msg << std::endl;
                if (msg == ":disconnect") {
                    emit("onDisconnected", socket, ip);
                } else if (msg.starts_with(":join")) {
                    size_t space = msg.find(' ');
                    if (space > msg.size()) { socket->SendMsg("Invalid :join syntax! Valid: \":join <roomId>\""); return; }
                    std::string room = msg;
                    room.erase(0, space+1);
                    int roomId = atoi(room.c_str());
                    std::cout << room << std::endl;
                    if (!RoomExists(roomId)) socket->SendMsg("Room with that ID doesn't exist!");
                    else JoinRoom(ip, roomId, socket);
                } else if (msg == ":createRoom") {
                    auto room = CreateRoom();
                    JoinRoom(ip, room.ID, socket);
                    std::string msg = std::string("Created room with id: ") + std::to_string(room.ID);
                    socket->SendMsg(msg.c_str());
                } else MessageAll(user.name + ": " + msg, socket);
            }
        });

        on("onDisconnected", [this](PURRNET_NS::Socket* socket, std::string data) {
            PURRNET_LOG_INF(PURRNET_FMT("[%s] Disconnected!", data.c_str()));
        });
    }

private:

    User &CreateUser(PURRNET_NS::Socket* sock) {
        int id = static_cast<int>(m_IpToID.size());
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

    void JoinRoom(std::string ip, int roomId, PURRNET_NS::Socket *socket) {
        auto id = m_IpToID[ip];
        socket->emit("joinRoom", socket, std::to_string(roomId));
        m_Users[id].roomId = roomId;
    }

    void SetUsersName(std::string ip, std::string name) {
        auto id = m_IpToID[ip];

        m_Users[id].name = name;
    }

    Room& CreateRoom() {
        Room room{};
        int id = static_cast<int>(m_Rooms.size());
        room.ID = id;
        if (m_Rooms.size() <= id) m_Rooms.resize(id+1);
        m_Rooms[id] = room;
        m_RoomExistanceTable[id] = true;
        return m_Rooms[id];
    }

    Room &GetRoom(int id) {
        if (m_Rooms.size() < id) return (Room&)Room();
        return m_Rooms[id];
    }

    bool RoomExists(int id) {
        if (m_RoomExistanceTable.find(id) == m_RoomExistanceTable.end()) m_RoomExistanceTable[id] = false;

        return m_RoomExistanceTable[id];
    }

    std::unordered_map<std::string, int> m_IpToID{};
    std::vector<User> m_Users{};
    std::vector<Room> m_Rooms{};
    std::unordered_map<int, bool> m_RoomExistanceTable{};

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