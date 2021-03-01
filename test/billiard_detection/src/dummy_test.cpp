#include <gtest/gtest.h>
#include <billiard_detection/billiard_detection.hpp>

TEST(Dummy, DummyFunc_getExpectedX) {
    // Act
    int result = billiard::detection::Dummy::getX();

    // Assert
    ASSERT_EQ(10, result);
}
