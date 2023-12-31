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

    virtual void Run() override {
        m_ReadThread = std::thread(&ExampleClient::ReadThread, this);
        while (m_Id == -1) {}
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        {
            std::cout << "What is your username? > ";
            std::string input = "";
            std::getline(std::cin, input);
            m_Socket->Send(input.data());
            system("cls");
            std::cout << "Welcome to SimpleChatApp v0.1 example." << std::endl;
            std::cout << "\ttype \"!disconnect\" to disconnect" << std::endl;
            std::cout << "\tby: RealCatDev" << std::endl << std::endl << std::endl << std::endl;
            m_ReadyForChat = true;

            for (auto& msg : m_Messages) std::cout << msg << std::endl;
            m_Messages.clear();
        }
        
        while (m_Running) {
            std::string input = "";
            std::getline(std::cin, input);
            m_Socket->Send(input.data());
            if (input == "!disconnect") m_Running = false;
        }

        if (m_ReadThread.joinable()) m_ReadThread.join();
    }

private:

    void ReadThread() {
        m_Id = *((uint32_t*) Read().data());
        PURRNET_LOG_INF(PURRNET_FMT("Assigned id: %llu", m_Id));
        while (m_Running) {
            auto msg = Read();
            if (!m_ReadyForChat) m_Messages.push_back(msg);
            else std::cout << msg << std::endl;
        }
    }

    std::thread m_ReadThread{};
    uint32_t m_Id = -1;
    bool m_ReadyForChat = false;
    std::vector<std::string> m_Messages{};

};

int main(int argc, char** argv) {
    if (!PurrfectNetworking::Initialize()) return 1;

    ExampleClient* client = nullptr;
    std::string input = "";
    bool running = true;

    try {
        client = new ExampleClient();
        client->Run();
    } catch (std::exception ex) {
        PURRNET_LOG_ERR(ex.what());
    }

    if (client) delete client;

    PurrfectNetworking::Shutdown();

    return 0;
}