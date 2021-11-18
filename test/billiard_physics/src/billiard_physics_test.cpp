#include <gtest/gtest.h>
#include <billiard_physics/billiard_physics.hpp>
#include <iostream>

void assertComplex(const std::vector<std::complex<double>>& expected, const std::vector<std::complex<double>>& actual);


TEST(frictionCoefficient, measurements) {

    double mass = 140.0 / 1000.0; // in kg
    double g = 9.81 * 1000.0; // mm/s^2

    struct Measurement {
        double h;
        double l;
        double rolledDistance;
        Measurement(double h, double l, double rolledDistance): h(h), l(l), rolledDistance(rolledDistance) {}
    };

    std::vector<Measurement> measurements = {
            // All lengths in Millimeters!
            Measurement { 13, 334.747666161, 941},
            Measurement { 19, 334.46076003, 1295},
    };

    double c_R_sum = 0.0;

    for (auto& measurement : measurements) {
        double h = measurement.h;
        double l = measurement.l;
        double rolledDistance = measurement.rolledDistance;

        double alpha = atan(h / l);
        double F_G = mass * g;
        double F_N = cos(alpha) * F_G;
        double F_H = sin(alpha) * F_G;
        double rampLength = sqrt(l*l + h*h);

        double rampTime = -1;
        {
            double a = 0.5 * F_H / mass;
            double b = 0.0;
            double c = -rampLength;
            auto result = billiard::physics::nonNegative(billiard::physics::intersection::solveQuadraticFormula(a, b, c));
            if (result.empty()) {
                std::cout << "Unable to find solution to ramp rolling time" << std::endl;
                return;
            }
            rampTime = result[0];
        }
        assert(rampTime > 0);

        double velocityAfterRamp = F_H / mass * rampTime;
        double v0 = velocityAfterRamp;

        double acceleration = (0.5 * (-v0)*(-v0) + v0 * (-v0)) / rolledDistance;

        double c_R = acceleration / g;

        std::cout << "Result: "
                  << "h=" << h << " "
                  << "l=" << l << " "
                  << "rolledDistance=" << rolledDistance << " "
                  << "mass=" << mass << " "
                  << "g=" << g << " "
                  << "alpha=" << alpha << " "
                  << "F_G=" << F_G << " "
                  << "F_N=" << F_N << " "
                  << "F_H=" << F_H << " "
                  << "rampLength=" << rampLength << " "
                  << "rampTime=" << rampTime << " "
                  << "velocityAfterRamp=" << velocityAfterRamp << " "
                  << "a=" << acceleration << " "
                  << "c_R=" << c_R << " "
                  << std::endl;

        c_R_sum += c_R;
    }

    double c_R_average = c_R_sum / measurements.size();
    std::cout << "Average c_R=" << c_R_average << std::endl;
}

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

TEST(solveQuarticFormula, shouldGetCorrectResult3) {
    std::vector<std::complex<double>> expected{
            0.11314,
            { 0.715435, 0.839049 },
            { 0.715435, -0.839049 },
            { 1.31773 }
    };

    double a = 0.009604;
    double b = -0.0274841;
    double c = 0.0327719;
    double d = -0.018757;
    double e = 0.00174089;

    // Act
    auto result = billiard::physics::intersection::solveQuarticFormula(a, b, c, d, e);

    // Assert
    assertComplex(expected, result);
}

TEST(timeToCollision, shouldGetCorrectResult) {
    glm::vec2 acceleration1 {94.7511, -171.576};
    glm::vec2 acceleration2 {-0, -0};
    glm::vec2 velocity1 {-67.7881, 122.751};
    glm::vec2 velocity2 {0, 0};
    glm::vec2 position1 {-868.681, 365.701};
    glm::vec2 position2 {-899.492, 425.088};
    float radius = 26.15f;
    float diameter = radius * 2;

    // Act
    auto result = billiard::physics::timeToCollision(acceleration1, velocity1, position1,
                                                     acceleration2, velocity2, position2,
                                                     diameter, radius);

    // Assert
    ASSERT_TRUE(result);
    ASSERT_NEAR(0.11314, *result, 0.0001f);
}

void assertComplex(const std::vector<std::complex<double>>& expected, const std::vector<std::complex<double>>& actual) {
    ASSERT_EQ(expected.size(), actual.size());
    for (int i = 0; i < actual.size(); i++) {
        ASSERT_NEAR(expected.at(i).real(), actual.at(i).real(),0.0001f);
        ASSERT_NEAR(expected.at(i).imag(), actual.at(i).imag(),0.0001f);
    }
}
