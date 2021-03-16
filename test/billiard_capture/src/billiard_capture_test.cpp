#include <gtest/gtest.h>
#include <billiard_capture/billiard_capture.hpp>

TEST(BilliardCapture, serveImage) {
    // Act
    auto image = billiard::capture::capture();

    // Assert
    ASSERT_TRUE(image.empty());
}