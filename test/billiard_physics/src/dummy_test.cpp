#include <gtest/gtest.h>
#include <billiard_physics/billiard_physics.hpp>

TEST(Dummy, DummyFunc_getExpectedX) {
    // Act
    int result = billiard::physics::Dummy::getX();

    // Assert
    ASSERT_EQ(10, result);
}