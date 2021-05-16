#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <billiard_detection/billiard_detection.hpp>

using billiard::detection::State;
using billiard::detection::Ball;
using billiard::detection::StateTracker;
using billiard::capture::ImageCapture;

class MockImageCapture : public ImageCapture {
public:
    MOCK_CONST_METHOD0(read, cv::Mat());
};

// TODO: fix test
//TEST(StateTracker, getExpectedStates) {
//    State expected{std::vector<Ball>{
//        Ball{glm::vec2{10, 10}, "TYPE", "ID"}
//    }};
//    auto imageCapture = std::make_shared<MockImageCapture>();
//    EXPECT_CALL(*imageCapture, read()).WillRepeatedly(::testing::Return(cv::Mat{}));
//
//    StateTracker tracker{imageCapture,
//                         [&expected](const State& prevState, const cv::Mat& image) -> State { return expected; }};
//    // Act
//    std::future<State> state1 = tracker.capture();
//    std::future<State> state2 = tracker.capture();
//
//    state1.wait();
//    state2.wait();
//
//    // Assert
//    ASSERT_EQ(expected, state1.get());
//    ASSERT_EQ(expected, state2.get());
//}
