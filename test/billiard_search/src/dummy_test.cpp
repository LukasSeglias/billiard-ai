#include <gtest/gtest.h>
#include <billiard_search/billiard_search.hpp>

TEST(Dummy, DummyFunc_getExpectedX) {
    // Act
    int result = billiard::search::Dummy::getX();

    // Assert
    ASSERT_EQ(10, result);
}