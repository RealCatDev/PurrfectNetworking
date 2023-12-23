#define PURRNET_USE_DP
#define PURRNET_DP 6969
#define PURRNET_MAXBUF 4096
#include <PurrfectNetworking.hpp>

static std::string Instructions = "\tWelcome to RoomDemo v1.0 by CatDev\nTo start type your username and press enter!\nTo join already existing room type `:join <roomId>`\nTo create new room type `:createRoom`\nTo disconnect type :disconnect";

class ExampleClient : public PURRNET_NS::Client {

public:

    ExampleClient()
        : PURRNET_NS::Client(std::string("127.0.0.1"), PURRNET_DP) {
    }

    ~ExampleClient() {

    }

    virtual void InitializeEvents() override {
        on("onMessage", [this](PURRNET_NS::Socket* socket, std::string data) {
            if (m_Id < 0) {
                std::cout << "ID: " << data << std::endl;
                m_Id = atoi(data.c_str());
                std::cout << Instructions << std::endl;
                std::cout << "Username: ";
            } else {
                std::cout << data << std::endl;
            }
        });

        on("joinRoom", [this](PURRNET_NS::Socket* socket, std::string data) {
            std::cout << "Join Room: " << data << std::endl;
        });
    }

    void HandleInput(std::string input) {
        Send(input);
        if (input == ":disconnect")
            m_Running = false;
    }

private:

    int m_Id = -1;
    bool m_ReadyForChat = false;
    std::vector<std::string> m_Messages{};

};

int main(int argc, char** argv) {
    if (!PurrfectNetworking::Initialize()) return 1;

    ExampleClient* client = nullptr;
    std::string input = "";
    bool running = true;

    client = new ExampleClient();
    client->Run();
    while (client->Running()) {
        std::string input;
        std::getline(std::cin, input);
        client->HandleInput(input);
    }

    if (client) delete client;

    PurrfectNetworking::Shutdown();

    return 0;
}