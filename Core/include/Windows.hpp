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

	inline static std::string hostname(std::string hostname) {

	}

	class WinSocket : public PURRNET_NS::Socket {

	public:

#if defined(PURRNET_USE_PC)
#if defined(PURRNET_USE_DP)
		inline WinSocket(std::string ip = "127.0.0.1")
			: Socket(PURRNET_DP) {
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
	}
#else
		inline WinSocket(int port, std::string ip = "127.0.0.1")
			: Socket(port) {
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

		inline WinSocket(SOCKET sock)
			: Socket(), m_Socket(sock) {
			
		}

		inline ~WinSocket() {
			closesocket(m_Socket);
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
			SOCKET sock = accept(m_Socket, NULL, NULL);
			if (sock == INVALID_SOCKET) {
				throw std::runtime_error(PURRNET_FMT("Failed to accept, error: %d!", WSAGetLastError()));
			}

			return new WinSocket(sock);
		}

		inline virtual void Connect(std::string ip, int port) override {
			sockaddr_in addr{};
			addr.sin_family = AF_INET;
			InetPtonW(AF_INET, StringToWideString(ip.data()).data(), &addr.sin_addr.s_addr);
			addr.sin_port = htons(port);

			if (connect(m_Socket, (const SOCKADDR*)&addr, sizeof(addr)) != 0) {
				throw std::runtime_error(PURRNET_FMT("Failed to connect to %s:%d!", ip.c_str(), port));
			}
		}

		inline virtual void Send(char* data) override {
			int byteCount = SOCKET_ERROR;
			if ((byteCount = send(m_Socket, data, PURRNET_MAXBUF, 0)) == SOCKET_ERROR)
				throw std::runtime_error(PURRNET_FMT("Failed to send data, error: %d!", WSAGetLastError()));
			else
				PURRNET_LOG_INF(PURRNET_FMT("Sent %ld bytes.", byteCount));
		}

		inline virtual RecieveData Recieve() override {
			RecieveData data{};
			int bytes = 0;
			if ((bytes = recv(m_Socket, data.buffer, PURRNET_MAXBUF, 0)) == SOCKET_ERROR)
				throw std::runtime_error(PURRNET_FMT("Failed to recieve data, error: %d!", WSAGetLastError()));
			return (data.size = bytes, data);
		}

	private:

		SOCKET m_Socket = INVALID_SOCKET;

	};

}

#endif
#endif