#ifndef PURRNET_EVENTS
#define PURRNET_EVENTS

#include "Core.hpp"
#include <functional>

namespace PURRNET_NS {

	namespace Events {

        class EventListener {
        public:
            using EventCallback = std::function<void(Socket*, std::string)>;

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

            void emit(const std::string& eventName, Socket* sock, std::string data) {
                if (m_Events[eventName]) m_Events[eventName](sock, data);
            }

        private:
            std::unordered_map<std::string, EventCallback> m_Events;
        };

	}

}

#endif