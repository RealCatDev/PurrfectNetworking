#pragma once

#include "Core.hpp"

// Include files for all platforms.
// Currenly supporting Windows only
#include "Windows.hpp"

namespace PURRNET_NS {

	class Server {

	public:

#ifndef PURRNET_USE_DP
		inline Server(int port)
#ifdef PURRNET_USE_PC
			: m_Socket(port)
#else
			: m_Socket()
#endif
		{
#ifndef PURRNET_USE_PC
			m_Socket->Bind("127.0.0.1", port);
#endif
		}

		inline Server(std::string ip, int port)
#ifdef PURRNET_USE_PC
			: m_Socket(new PURRNET_NS::SOCKET_CLASS(port))
#else
			: m_Socket(new PURRNET_NS::SOCKET_CLASS())
#endif
		{
#ifndef PURRNET_USE_PC
			m_Socket->Bind(ip, port);
#endif
		}
#else
		inline Server()
#ifdef PURRNET_USE_PC
			: m_Socket(new PURRNET_NS::SOCKET_CLASS(PURRNET_DP))
#else
			: m_Socket(new PURRNET_NS::SOCKET_CLASS())
#endif
		{
#ifndef PURRNET_USE_PC
			m_Socket->Bind("127.0.0.1", PURRNET_DP);
#endif
		}

		inline Server(std::string ip, int port)
#ifdef PURRNET_USE_PC
			: m_Socket(new PURRNET_NS::SOCKET_CLASS(port))
#else
			: m_Socket(new PURRNET_NS::SOCKET_CLASS())
#endif
		{
#ifndef PURRNET_USE_PC
			m_Socket->Bind(ip, port);
#endif
		}
#endif

		inline ~Server() {
			delete m_Socket;
		}

		inline void Run() {
			m_Socket->Listen();
			m_ListenThread = std::thread(&Server::ListenerThread, this);
		}

		inline void Stop() { 
			m_Running = false;
			if (m_ListenThread.joinable()) m_ListenThread.join();
			for (auto &thread : m_Threads) if (thread.joinable()) thread.join();
		}

		inline bool Running() { return m_Running; }

	protected:

		bool m_Running = true;

	private:

		inline void ListenerThread() {
			while (m_Running) {
				try {
					m_Threads.emplace_back(&PURRNET_NS::Server::ClientThread, this, m_Socket->AcceptSocket());
				} catch (std::exception ex) {
					PURRNET_LOG_ERR(ex.what());
					m_Running = false;
				}
			}
		}

		virtual void ClientThread(PURRNET_NS::Socket *sock) = 0;

		std::thread m_ListenThread{};
		std::vector<std::thread> m_Threads{};
		Socket *m_Socket = nullptr;

	};

	class Client {

	public:

#ifndef PURRNET_USE_DP
		inline Client(std::string ip, int port)
#ifdef PURRNET_USE_PC
			: m_Socket(port, ip)
#else
			: m_Socket()
#endif
		{
#ifndef PURRNET_USE_PC
			m_Socket->Bind(ip, port);
#endif
		}
#endif

		inline ~Client() {
			delete m_Socket;
		}

		inline void Send(std::string msg) {
			m_Socket->Send((char*)msg.data());
		}

		template <typename T>
		inline void Send(T *msg) {
			m_Socket->Send((char*)msg);
		}

		inline std::string Read() {
			auto data = m_Socket->Recieve();

			return std::string(data.buffer, data.size);
		}

		template <typename T>
		inline T *Read() {
			auto data = m_Socket->Recieve();

			return (T*)data;
		}

	private:

		Socket* m_Socket = nullptr;

	};

}