#include <billiard_detection/billiard_detection.hpp>

billiard::detection::StateTracker::StateTracker(const std::shared_ptr<capture::ImageCapture>& imageCapture,
                                                const std::function<State(const State& previousState,
                                                                          const cv::Mat&)>& detect)
        :
        _exitSignal(),
        _lock(),
        _waiting(),
        _thread(std::thread(StateTracker::work, _exitSignal.get_future(), std::ref(_lock), imageCapture, detect,
                            std::ref(_waiting))) {
}

billiard::detection::StateTracker::~StateTracker() {
    _exitSignal.set_value();
    _thread.join();
}

std::future<billiard::detection::State> billiard::detection::StateTracker::capture() {
    std::promise<State> promise;
    auto future = promise.get_future();
    _lock.lock();
    _waiting.emplace(std::move(promise));
    _lock.unlock();
    return future;
}

void billiard::detection::StateTracker::work(std::future<void> exitSignal,
          std::mutex& lock,
          const std::shared_ptr<capture::ImageCapture>& imageCapture,
          const std::function<State (const State& previousState, const cv::Mat&)>& detect,
          std::queue<std::promise<State>>& waiting) {
    State previousState;
    // TODO: Initialize auruco-marker, internal state matrices
    while (exitSignal.wait_for(std::chrono::nanoseconds (10)) == std::future_status::timeout) {
        auto image = imageCapture->read();
        auto state = detect(previousState, image);
        previousState = state;
        lock.lock();
        if (!waiting.empty()) {
            // TODO: Convert state to internal coordinates
            while (!waiting.empty()) {
                auto& prom = waiting.front();
                prom.set_value(state);
                waiting.pop();
            }
        }
        lock.unlock();
    }
}

