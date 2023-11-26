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
            std::cout << "\tby: RealCatDev" << std::endl << std::endl << std::endl << std::endl;
        }
        
        while (m_Running) {
            std::string input = "";
            std::getline(std::cin, input);
            if (input == "!quit") Stop();
            else m_Socket->Send(input.data());
        }

        if (m_ReadThread.joinable()) m_ReadThread.join();
    }

private:

    void ReadThread() {
        m_Id = *((uint32_t*) Read().data());
        PURRNET_LOG_INF(PURRNET_FMT("Assigned id: %llu", m_Id));
        while (m_Running) {
            auto msg = Read();
            std::cout << msg << std::endl;
        }
    }

    std::thread m_ReadThread{};
    uint32_t m_Id = -1;

};

int main(int argc, char** argv) {
    if (!PurrfectNetworking::Initialize()) return 1;

    ExampleClient* client = nullptr;
    std::string input = "";
    bool running = true;

    try {
        client = new ExampleClient();
        client->Run();
        while(client->Running()) {}
    } catch (std::exception ex) {
        PURRNET_LOG_ERR(ex.what());
        goto cleanup;
    }

cleanup:
    if (client) delete client;

    PurrfectNetworking::Shutdown();

    return 0;
}