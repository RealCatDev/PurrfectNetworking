#ifndef PURRNET_CORE
#define PURRNET_CORE

#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <unordered_map>

#ifndef PURRNET_NS
// PURRNET (PurrfectNetworking) _NS (_NameSpace)
#define PURRNET_NS PurrfectNetworking
#endif

#ifndef PURRNET_DP
// PURRNET (PurrfectNetworking) _DP (_DefaultPort)
#define PURRNET_DP 2346
#endif

// PURRNET (PurrfectNetworking)_USE_* change functions based on that
// Example usage:
//   - #define PURRNET_USE_DP
//                            : Removes port parameter from Listen()
// List of all USE_ defines:
//   - ...DP (Default Port)
//   - ...PC (Port Constructor) - Use port in constructor
//
//   ... = PURRNET_USE_<name>

#ifndef PURRNET_LOG_INF
#define PURRNET_LOG_INF(msg) std::cout << "[PurrfectNetworking] (info): " << msg << std::endl;
#endif

#ifndef PURRNET_LOG_ERR
#define PURRNET_LOG_ERR(msg) std::cerr << "[PurrfectNetworking] (error): " << msg << std::endl;
#endif

#ifndef PURRNET_MAXBUF
#define PURRNET_MAXBUF 256
#endif

#ifndef PURRNET_FMT
#include <cstdio>
#include <cstdarg>
#include <functional>

const char* PURRNET_FMT(const char* fmt, ...) {
	char* buffer = (char*)malloc(PURRNET_MAXBUF);
	if (buffer != nullptr) {
		va_list args;
		va_start(args, fmt);
		vsnprintf(buffer, PURRNET_MAXBUF, fmt, args);
		va_end(args);
	}
	return buffer;
}
#endif

namespace PURRNET_NS {

	class ClientDisconnectedException : public std::exception {
	public:
		inline ClientDisconnectedException() {}
		inline ~ClientDisconnectedException() {}
	private:
	};

	class Socket;

	namespace Events {

		class EventListener {
		public:
			using EventCallback = std::function<void(PURRNET_NS::Socket*, std::string)>;

			void on(const std::string& eventName, const EventCallback& callback) {
				if (!m_Events[eventName]) m_Events[eventName] = (callback);
			}

			void once(const std::string& eventName, const EventCallback& callback) {
				auto onceWrapper = [this, eventName, callback](Socket* sock, std::string data) {
					callback(sock, data);
					off(eventName, callback);
					};

				on(eventName, onceWrapper);
			}

			void off(const std::string& eventName, const EventCallback& callback) {
				if (m_Events[eventName]) m_Events[eventName] = nullptr;
			}

			virtual void emit(const std::string& eventName, Socket* sock, std::string data) {
				if (m_Events[eventName]) m_Events[eventName](sock, data);
			}

		private:
			std::unordered_map<std::string, EventCallback> m_Events;
		};

	}

	struct RecieveData {
		char buffer[PURRNET_MAXBUF] = { 0 };
		size_t size = 0;
	};

	static bool Initialize();

	// Used to convert hostname to ip
	static std::string hostname(std::string hostname); 

	class Socket : public Events::EventListener {

	public:

#ifdef PURRNET_USE_PC
		inline Socket(int port) 
		{
		}
#else
		inline Socket()
		{
		}

		virtual void Bind(std::string ip, int port) = 0;
#endif

		inline ~Socket() {

		}

		virtual void Listen() = 0;
		virtual Socket *AcceptSocket() = 0;
		
		virtual void Connect(std::string ip, int port) = 0;

		virtual void Send(const char *data) = 0;
		virtual RecieveData Recieve() = 0;

		virtual std::string GetIpAddress() const = 0;

	private:

	};

}

#endif