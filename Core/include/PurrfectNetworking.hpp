#pragma once

#include "Core.hpp"

// Include files for all platforms.
// Currenly supporting Windows only
#include "Windows.hpp"

namespace PURRNET_NS {

	class Client {

	public:

#ifndef PURRNET_USE_DP
		inline Client(int port)
#ifdef PURRNET_USE_PC
			: m_Socket(new PURRNET_NS::SOCKET_CLASS(port, std::string("127.0.0.1"), false))
#else
			: m_Socket(new PURRNET_NS::SOCKET_CLASS(std::string("127.0.0.1"), false))
#endif
		{
#ifndef PURRNET_USE_PC
			m_Socket->Connect("127.0.0.1", port);
#endif
		}

		inline Client(std::string ip, int port)
#ifdef PURRNET_USE_PC
			: m_Socket(new PURRNET_NS::SOCKET_CLASS(port))
#else
			: m_Socket(new PURRNET_NS::SOCKET_CLASS())
#endif
		{
#ifndef PURRNET_USE_PC
			m_Socket->Connect(ip, port);
#endif
		}
#else
		inline Client()
#ifdef PURRNET_USE_PC
			: m_Socket(new PURRNET_NS::SOCKET_CLASS(PURRNET_DP))
#else
			: m_Socket(new PURRNET_NS::SOCKET_CLASS())
#endif
		{
#ifndef PURRNET_USE_PC
			m_Socket->Connect("127.0.0.1", PURRNET_DP);
#endif
		}

		inline Client(std::string ip, int port)
#ifdef PURRNET_USE_PC
			: m_Socket(new PURRNET_NS::SOCKET_CLASS(port))
#else
			: m_Socket(new PURRNET_NS::SOCKET_CLASS())
#endif
		{
#ifndef PURRNET_USE_PC
			m_Socket->Connect(ip, port);
#endif
		}
#endif

		inline Client(PURRNET_NS::Socket* socket)
			: m_Socket(socket) {
		}

		inline ~Client() {
			delete m_Socket;
		}

		inline virtual void Run() {

		}

		inline bool Running() { return m_Running; }

		inline void Send(std::string msg) {
			m_Socket->Send(msg.data());
		}

		template <typename T>
		inline void Send(T* msg) {
			m_Socket->Send((char*)msg);
		}

		inline std::string Read() {
			auto data = m_Socket->Recieve();

			return std::string(data.buffer, data.size);
		}

		template <typename T>
		inline T* Read() {
			auto data = m_Socket->Recieve();

			return (T*)data;
		}

		inline void Stop() { m_Running = false; }

	protected:

		bool m_Running = true;
		Socket* m_Socket = nullptr;

	};

	class Server {

	public:

#ifndef PURRNET_USE_DP
		inline Server(int port)
#ifdef PURRNET_USE_PC
			: m_Socket(new PURRNET_NS::SOCKET_CLASS(port))
#else
			: m_Socket(new PURRNET_NS::SOCKET_CLASS())
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
			m_DeletionThread = std::thread(&Server::DeletionThread, this);
			m_ListenThread = std::thread(&Server::ListenerThread, this);
		}

		inline void Stop() { 
			m_Running = false;
			if (m_ListenThread.joinable()) m_ListenThread.join();
			for (auto &pair : m_Clients) if (pair.second.joinable()) pair.second.join();
		}

		inline bool Running() { return m_Running; }

		inline void MessageAll(std::string message) {
			for (const auto& pair : m_Clients) {
				pair.first->Send(message.data());
			}
		}

		inline void MessageAll(std::string message, PURRNET_NS::Socket *socket) {
			for (const auto& pair : m_Clients) {
				if (pair.first != socket) pair.first->Send(message.data());
			}
		}

	protected:

		bool m_Running = true;

		inline void DeleteClient(PURRNET_NS::Socket *socket) {
			m_SocketToDelete = socket;
		}

	private:

		inline void ListenerThread() {
			while (m_Running) {
				try {
					auto socket = m_Socket->AcceptSocket();
					m_Clients[socket] = std::thread(&PURRNET_NS::Server::ClientThread, this, socket);
				} catch (std::exception ex) {
					PURRNET_LOG_ERR(ex.what());
					m_Running = false;
				}
			}
		}

		inline void DeletionThread() {
			while (1) {
				if (m_SocketToDelete != nullptr) {
					if (m_Clients[m_SocketToDelete].joinable()) m_Clients[m_SocketToDelete].join();
					m_Clients.erase(m_SocketToDelete);
					m_SocketToDelete = nullptr;
				}
			}
		}

		virtual void ClientThread(PURRNET_NS::Socket *socket) = 0;

		std::thread m_ListenThread{};
		std::thread m_DeletionThread{};
		std::unordered_map<PURRNET_NS::Socket *, std::thread> m_Clients{};
		Socket *m_SocketToDelete = nullptr;
		Socket *m_Socket = nullptr;

	};

}