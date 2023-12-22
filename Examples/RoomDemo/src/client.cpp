#define PURRNET_USE_DP
#define PURRNET_DP 6969
#define PURRNET_MAXBUF 4096
#include <PurrfectNetworking.hpp>

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
                m_Id = atoi(data.c_str());
            } else {
                std::cout << data << std::endl;
            }
        });
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
        
    }

    if (client) delete client;

    PurrfectNetworking::Shutdown();

    return 0;
}