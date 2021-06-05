#include <gtest/gtest.h>
#include <glm/glm.hpp>

void error(const glm::vec2& M, const glm::vec2& T, const glm::vec2& K, const glm::vec2& F, float R) {

    glm::vec2 P = M + 2*R * glm::normalize(K-M);
    glm::vec2 Me = M + F;
    glm::vec2 Pe = Me + 2*R * glm::normalize(Me-T);

    {
        glm::vec2 V = glm::normalize(Pe - K);
        glm::vec2 P = K;
        glm::vec2 C = M;

        float a = 1;
        float b = 2.0f * glm::dot(V, P-C);
        float c = glm::dot(P-C, P-C) - glm::pow((2*R), 2);
        float discriminant = b*b - 4*a*c;
        if (discriminant < 0) {
            std::cout << "Kein Schnittpunkt" << std::endl;
        } else {
            float t1 = (-b + sqrt(discriminant)) / 2*a;
            float t2 = (-b - sqrt(discriminant)) / 2*a;
            float t = glm::min(t1, t2);
            glm::vec2 Pee = K + t * glm::normalize(Pe - K);
            float beta = glm::acos(glm::dot(glm::normalize(Pee - M), glm::normalize(P - M)));

            float D = glm::length(T - M);
            float d = D * glm::tan(beta);

            std::cout << "Beta: " << beta << " rad, d: " << d << std::endl;
        }

    }
}

TEST(Error, max) {

    glm::vec2 M {100,0};
    glm::vec2 T {1800,0};
    glm::vec2 K {0,0};
    glm::vec2 F {0,2.15};
    float R = 26.15;

    error(M, T, K, F, R);
}

TEST(Error, realistic) {

    glm::vec2 M {100,0};
    glm::vec2 T {950,0};
    glm::vec2 K {0,0};
    glm::vec2 F {0,2.15};
    float R = 26.15;

    error(M, T, K, F, R);
}
