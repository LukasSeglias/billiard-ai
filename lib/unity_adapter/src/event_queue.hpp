#pragma once

#include <queue>
#include <mutex>
#include <memory>

namespace unity {
    template <class Event, class Callback>
    class EventQueue {
    public:
        void callback(Callback callback) {
            _guard.lock();
            this->_callback = callback;
            _guard.unlock();
        }

        void push(std::shared_ptr<Event> event) {
            _guard.lock();
            _queue.push(event);
            _guard.unlock();
        }

        void process() {
            _guard.lock();

            if (!_queue.empty() && _callback != nullptr) {
                auto oldest = _queue.front();
                _callback(*oldest);
                _queue.pop();
            }

            _guard.unlock();
        }

    private:
        std::mutex _guard;
        std::queue<std::shared_ptr<Event>> _queue;

        Callback _callback;
    };
}