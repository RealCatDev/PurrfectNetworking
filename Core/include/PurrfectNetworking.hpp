#pragma once

#include "Core.hpp"

// Include files for all platforms.
// Currenly supporting Windows only
#include "Windows.hpp"

#include <sstream>

namespace PURRNET_NS {

	class Client : public Events::EventListener {

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

		inline void Run() {
			InitializeEvents();
			m_ListenerThread = std::thread(&Client::ListenerThread, this);
		}

		inline bool Running() { return m_Running; }

		inline void Send(std::string msg) {
			m_Socket->Send(msg.data());
		}

		template <typename T>
		inline void Send(T* msg) {
			m_Socket->Send((const char*)msg);
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

		virtual void InitializeEvents() {

		}

		inline void ListenerThread() {
			char buf[PURRNET_MAXBUF] = { 0 };
			int bytesRead = 0;
			std::string ip = m_Socket->GetIpAddress();

			emit("onConnected", m_Socket, ip);

			while (m_Running && m_Socket != nullptr) {
				try {
					auto data = m_Socket->Recieve();
					auto input = std::string(data.buffer);
					if (input.find("evt") == 0) {
						std::string name, data;
						if (!parseEventString(input, name, data)) { Send("Invalid event string!"); }
						emit(name, m_Socket, data);
					} else {
						std::string msg = input.erase(0, input.find(' ')+1);

						emit("onMessage", m_Socket, msg);
					}
				} catch (PURRNET_NS::ClientDisconnectedException& ex) {
					break;
				} catch (std::exception& ex) {
					PURRNET_LOG_ERR(ex.what());
					m_Running = false;
					break;
				}
			}

			emit("onDisconnected", m_Socket, ip);
		}

		bool m_Running = true;
		Socket* m_Socket = nullptr;

	private:

		bool parseEventString(const std::string& input, std::string &eventName, std::string &eventData) {
			std::istringstream iss(input);
			std::string command;
			if (!(iss >> command) || command != "evt") return false;
			if (!(iss >> eventName) || !(std::getline(iss, eventData))) return false;
			eventData = eventData.substr(eventData.find_first_not_of(" \t"));
			
			return true;
		}

		std::thread m_ListenerThread{};

	};

	class Server : public Events::EventListener {

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
			InitializeEvents();
			m_Socket->Listen();
			m_DeletionThread = std::thread(&Server::DeletionThread, this);
			m_DeletionThread.detach();
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
				pair.first->SendMsg(message.data());
			}
		}

		inline void MessageAll(std::string message, PURRNET_NS::Socket *socket) {
			for (const auto& pair : m_Clients) {
				if (pair.first && pair.first != socket) pair.first->SendMsg(message.data());
			}
		}

		inline void MessageIf(std::string message, std::function<bool(PURRNET_NS::Socket*)> func) {
			for (const auto& pair : m_Clients) {
				if (func(pair.first)) pair.first->SendMsg(message.data());
			}
		}

	protected:

		bool m_Running = true;
		std::mutex m_Mutex;

		inline void DeleteClient(PURRNET_NS::Socket *socket) {
			m_SocketToDelete = socket;
		}

		virtual void InitializeEvents() {}

	private:

		void ListenerThread() {
			while (m_Running) {
				try {
					auto socket = m_Socket->AcceptSocket();
					if (socket != nullptr) {
						std::lock_guard<std::mutex> lock(m_Mutex);
						m_Clients[socket] = std::thread(&Server::ClientThread, this, socket);
					}
				}
				catch (const std::exception& ex) {
					PURRNET_LOG_ERR(ex.what());
					m_Running = false;
				}
			}
		}

		void DeletionThread() {
			while (m_Running) {
				Socket* socketToDelete = nullptr;
				{
					std::lock_guard<std::mutex> lock(m_Mutex);
					socketToDelete = m_SocketToDelete;
					m_SocketToDelete = nullptr;
				}

				if (socketToDelete != nullptr) {
					{
						std::lock_guard<std::mutex> lock(m_Mutex);
						if (m_Clients.find(socketToDelete) != m_Clients.end()) {
							auto& clientThread = m_Clients[socketToDelete];
							if (clientThread.joinable()) {
								clientThread.join();
							}
							m_Clients.erase(socketToDelete);
						}
					}

					delete socketToDelete;
				}
			}
		}

		void ClientThread(PURRNET_NS::Socket* socket) {
			char buf[PURRNET_MAXBUF] = { 0 };
			int bytesRead = 0;
			std::string ip = socket->GetIpAddress();

			emit("onConnected", socket, ip);

			while (m_Running && socket != nullptr) {
				try {
					auto data = socket->Recieve();

					emit("onMessage", socket, ip + "|" + data.buffer);
				} catch (PURRNET_NS::ClientDisconnectedException ex) {
					break;
				} catch (std::exception ex) {
					PURRNET_LOG_ERR(ex.what());
					break;
					m_Running = false;
				}
			}

			emit("onDisconnected", socket, ip);
			DeleteClient(socket);
		}

		std::thread m_ListenThread{};
		std::thread m_DeletionThread{};
		std::unordered_map<PURRNET_NS::Socket *, std::thread> m_Clients{};
		Socket *m_SocketToDelete = nullptr;
		Socket *m_Socket = nullptr;

	};

}