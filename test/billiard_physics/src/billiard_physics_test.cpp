#include <gtest/gtest.h>
#include <billiard_physics/billiard_physics.hpp>
#include <iostream>

void assertComplex(const std::vector<std::complex<double>>& expected, const std::vector<std::complex<double>>& actual);

TEST(solveQuarticFormula, shouldGetCorrectResult) {
    std::vector<std::complex<double>> expected{
            -1.1753f,
            -0.12603f,
            2.1260f,
            3.1753f
    };
    double a = 1;
    double b = -4;
    double c = 0;
    double d = 8;
    double e = 1;

    // Act
    auto result = billiard::physics::intersection::solveQuarticFormula(a, b, c, d, e);

    // Assert
    assertComplex(expected, result);
}

TEST(solveQuarticFormula, shouldGetCorrectResult2) {
    std::vector<std::complex<double>> expected{
            -0.91904,
            -0.13283,
            { 2.5259, -1.3459 },
            { 2.5259, 1.3459 }
    };
    double a = 1;
    double b = -4;
    double c = 3;
    double d = 8;
    double e = 1;

    // Act
    auto result = billiard::physics::intersection::solveQuarticFormula(a, b, c, d, e);

    // Assert
    assertComplex(expected, result);
}

void assertComplex(const std::vector<std::complex<double>>& expected, const std::vector<std::complex<double>>& actual) {
    ASSERT_EQ(expected.size(), actual.size());
    for (int i = 0; i < actual.size(); i++) {
        ASSERT_NEAR(expected.at(i).real(), actual.at(i).real(),0.0001f);
        ASSERT_NEAR(expected.at(i).imag(), actual.at(i).imag(),0.0001f);
    }
}
