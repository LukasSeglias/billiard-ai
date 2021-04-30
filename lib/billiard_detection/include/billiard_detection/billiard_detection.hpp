#pragma once

#include <billiard_capture/billiard_capture.hpp>
#include <future>
#include <thread>
#include <mutex>
#include <memory>
#include <functional>
#include <queue>

#include "macro_definition.hpp"
#include "type.hpp"


namespace billiard::detection {

    class EXPORT_BILLIARD_DETECTION_LIB StateTracker {
    public:
        StateTracker(const std::shared_ptr<capture::ImageCapture>& imageCapture,
                     const std::function<State(const State& previousState, const cv::Mat&)>& detect); // TODO: Add Configuration for aruco initialization
        ~StateTracker();

        std::future<State> capture();

    private:
        std::promise<void> _exitSignal;
        std::mutex _lock;
        std::queue<std::promise<State>> _waiting;
        std::thread _thread;

        static void work(std::future<void> exitSignal,
                         std::mutex& lock,
                         const std::shared_ptr<capture::ImageCapture>& imageCapture,
                         const std::function<State (const State& previousState, const cv::Mat&)>& detect,
                         std::queue<std::promise<State>>& waiting); // TODO: Pass configuration for auruco configuration
    };
}