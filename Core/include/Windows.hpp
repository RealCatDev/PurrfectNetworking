#ifndef PURRNET_WINDOWS
#define PURRNET_WINDOWS

#include "Core.hpp"

#ifdef _WIN32

#define PURRNET_WINDOWS

#pragma comment(lib, "Ws2_32.lib")

#include <tchar.h>
#define _WINSOCKAPI_
#include <Windows.h>
#include <WinSock2.h>
#include <Ws2tcpip.h>

namespace PURRNET_NS {

#define SOCKET_CLASS WinSocket

	inline std::wstring StringToWideString(const std::string& narrowStr) {
		int bufferSize = MultiByteToWideChar(CP_UTF8, 0, narrowStr.c_str(), -1, nullptr, 0);
		if (bufferSize == 0) return L"";
		std::vector<wchar_t> wideStr(bufferSize);
		if (MultiByteToWideChar(CP_UTF8, 0, narrowStr.c_str(), -1, wideStr.data(), bufferSize) == 0) return L"";
		return std::wstring(wideStr.data());
	}

	static WSADATA s_WsaData{};

	inline static bool Initialize() {
		int err = 0;
		if ((err = WSAStartup(MAKEWORD(2, 2), &s_WsaData)) != 0) {
			PURRNET_LOG_ERR("Failed! WinSock dll not found!");
			return false;
		}
		PURRNET_LOG_INF(PURRNET_FMT("WinSock status: %d", s_WsaData.szSystemStatus));

		return true;
	}

	inline static void Shutdown() {
		WSACleanup();
		
		PURRNET_LOG_INF("PurrfectNetworking Shuted down!");
	}

	inline static std::string hostname(const char* hostname) {
		addrinfo hints;
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;      // Use IPv4
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		addrinfo* result = nullptr;
		if (getaddrinfo(hostname, nullptr, &hints, &result) != 0) return {};

		char ip[INET_ADDRSTRLEN] = {0};
		for (addrinfo* ptr = result; ptr != nullptr; ptr = ptr->ai_next) {
			if (ptr->ai_family == AF_INET) {
				sockaddr_in* ipv4 = reinterpret_cast<sockaddr_in*>(ptr->ai_addr);
				inet_ntop(AF_INET, &(ipv4->sin_addr), ip, INET_ADDRSTRLEN);
				PURRNET_LOG_INF(PURRNET_FMT("%s", ip));
			}
		}

		freeaddrinfo(result);
		return ip;
	}

	class WinSocket : public PURRNET_NS::Socket {

	public:

#if defined(PURRNET_USE_PC)
#if defined(PURRNET_USE_DP)
		inline WinSocket(std::string ip = "127.0.0.1", bool server = true)
			: Socket(PURRNET_DP) {
			if (server) {
				m_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
				if (m_Socket == INVALID_SOCKET) {
					// TODO: Create new kind of error: SocketErr
					throw std::runtime_error(PURRNET_FMT("Failed to create Socket, error: %d!", WSAGetLastError()));
				}

				sockaddr_in addr{};
				addr.sin_family = AF_INET;
				addr.sin_port = htons(port);
				IInetPtonW(AF_INET, StringToWideString(ip.data()).data(), &addr.sin_addr.s_addr);

				if (bind(m_Socket, (const sockaddr*)&addr, sizeof(addr)) != 0) {
					throw std::runtime_error(PURRNET_FMT("Failed to bind Socket, error: %d!", WSAGetLastError()));
				}
			} else {
				Connect(ip, PURRNET_DP);
			}
	}
#else
		inline WinSocket(int port, std::string ip = "127.0.0.1", bool server = true)
			: Socket(port) {
			if (server) {
				m_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
				if (m_Socket == INVALID_SOCKET) {
					// TODO: Create new kind of error: SocketErr
					throw std::runtime_error(PURRNET_FMT("Failed to create Socket, error: %d!", WSAGetLastError()));
				}

				sockaddr_in addr{};
				addr.sin_family = AF_INET;
				addr.sin_port = htons(port);
				InetPtonW(AF_INET, StringToWideString(ip.data()).data(), &addr.sin_addr.s_addr);

				if (bind(m_Socket, (const sockaddr*)&addr, sizeof(addr)) != 0) {
					throw std::runtime_error(PURRNET_FMT("Failed to bind Socket, error: %d!", WSAGetLastError()));
				}
			} else {
				Connect(ip, port);
			}
		}
#endif
#else
		inline WinSocket()
			: Socket() {
			m_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (m_Socket == INVALID_SOCKET) {
				// TODO: Create new kind of error: SocketErr
				throw std::runtime_error(PURRNET_FMT("Failed to create Socket, reason: %d!", WSAGetLastError()));
			}
		}

		inline virtual void Bind(std::string ip, int port) override {
			sockaddr_in addr{};
			addr.sin_family = AF_INET;
			addr.sin_port = htons(port);
			InetPtonW(AF_INET, StringToWideString(ip.data()).data(), &addr.sin_addr.s_addr);

			if (bind(m_Socket, (const sockaddr*)&addr, sizeof(addr)) != 0) {
				throw std::runtime_error(PURRNET_FMT("Failed to bind Socket, error: %d!", WSAGetLastError()));
			}
		}
#endif

		inline WinSocket(SOCKET sock, sockaddr_in addr, int addrSize)
			: Socket(), m_SockInfo(addr), m_SockInfoSize(addrSize), m_Socket(sock) {
		}

		inline ~WinSocket() {
			if (m_Socket != INVALID_SOCKET) closesocket(m_Socket);
		}

		inline virtual void Listen() override {
			if (listen(m_Socket, 1) != 0) {
				throw std::runtime_error(PURRNET_FMT("Failed to listen, error: %d!", WSAGetLastError()));
			}
			else {
				PURRNET_LOG_INF("Listening...");
			}
		}

		inline virtual Socket* AcceptSocket() override {
			sockaddr_in addr{};
			int addrSize = sizeof(addr);
			SOCKET sock = accept(m_Socket, (struct sockaddr*) &addr, &addrSize);
			if (sock == INVALID_SOCKET) {
				throw std::runtime_error(PURRNET_FMT("Failed to accept, error: %d!", WSAGetLastError()));
			}

			auto socket = new WinSocket(sock, addr, addrSize);
			return socket;
		}

		inline virtual void Connect(std::string ip, int port) override {
			sockaddr_in addr{};
			addr.sin_family = AF_INET;
			InetPtonW(AF_INET, StringToWideString(ip.data()).data(), &addr.sin_addr.s_addr);
			addr.sin_port = htons(port);
			if (connect(m_Socket, (const SOCKADDR*)&addr, sizeof(addr)) != 0)
				throw std::runtime_error(PURRNET_FMT("Failed to connect to %s:%d!", ip.c_str(), port));
			else PURRNET_LOG_INF(PURRNET_FMT("Connected to %s:%d!", ip.data(), port));
		}

		inline virtual void Send(const char* data) override {
			if (m_Socket == INVALID_SOCKET) throw std::runtime_error("Invalid socket!");
			int byteCount = SOCKET_ERROR;
			int dataSize = static_cast<int>(strlen(data) + 1);
			if ((byteCount = send(m_Socket, data, dataSize, 0)) == SOCKET_ERROR)
				throw PURRNET_NS::ClientDisconnectedException();
#ifdef PURRNET_LALL
			else
				PURRNET_LOG_INF(PURRNET_FMT("Sent %ld bytes.", byteCount));
#endif
		}
		
		inline virtual RecieveData Recieve() override {
			RecieveData data{};
			int bytes = 0;
			if ((bytes = recv(m_Socket, data.buffer, PURRNET_MAXBUF, 0)) == SOCKET_ERROR)
				throw PURRNET_NS::ClientDisconnectedException(); // PURRNET_FMT("Failed to recieve data, error: %d!", WSAGetLastError()) - previous error message (got replaced with exception)
			return (data.size = bytes, data);
		}
		
		inline virtual std::string GetIpAddress() const override {
			return std::string(inet_ntoa(m_SockInfo.sin_addr)) + ":" + std::to_string(m_SockInfo.sin_port);
		}

	private:
		sockaddr_in m_SockInfo{};
		int m_SockInfoSize = 0;
		SOCKET m_Socket = INVALID_SOCKET;
	};

}

#endif
#endif