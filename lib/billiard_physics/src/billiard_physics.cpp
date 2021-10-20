#include <billiard_physics/billiard_physics.hpp>
#include <billiard_debug/billiard_debug.hpp>
#include <ostream>
#include <cmath>

std::ostream& operator<<(std::ostream& os, const glm::vec2& vector){
    os << "(" << vector.x << ", " << vector.y << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const std::vector<float>& values){
    os << "vector { ";
    for (auto value : values) {
        os << "" << value << ", ";
    }
    os << " }";
    return os;
}

namespace billiard::physics::intersection {
    std::vector<std::complex<double>> solveQuartic(std::complex<double> firstPart, std::complex<double> discriminant);
    std::vector<float> realRoot(const std::vector<std::complex<double>>& roots);
}

float billiard::physics::pointToLineSegmentSquaredDistance(const glm::vec2& linePoint1, const glm::vec2& linePoint2, const glm::vec2& point) {
    glm::vec2 lineDirection = linePoint2 - linePoint1;
    glm::vec2 zero{0, 0};
    assert(lineDirection != zero);
    glm::vec2 lineDirectionNormalized = glm::normalize(lineDirection);
    glm::vec2 toPoint = point - linePoint1;

    float factor = glm::dot(lineDirectionNormalized, toPoint) / glm::length(lineDirection);

    if (factor < 0) {
        // Return squared distance between point and start-point of line segment
        glm::vec2 startPointToPoint = point - linePoint1;
        return glm::dot(startPointToPoint, startPointToPoint);
    }
    if (factor > 1.0) {
        // Return squared distance between point and end-point of line segment
        glm::vec2 endPointToPoint = point - linePoint2;
        return glm::dot(endPointToPoint, endPointToPoint);
    }

    glm::vec2 distanceVector = toPoint - (lineDirection * factor);
    return glm::dot(distanceVector, distanceVector);
}

float billiard::physics::pointToHalfLineSquaredDistance(const glm::vec2& linePoint1, const glm::vec2& lineDirection, const glm::vec2& point) {
    static glm::vec2 zero{0, 0};
    assert(lineDirection != zero);
    glm::vec2 lineDirectionNormalized = glm::normalize(lineDirection);
    glm::vec2 toPoint = point - linePoint1;

    float factor = glm::dot(lineDirectionNormalized, toPoint) / glm::length(lineDirection);

    if (factor < 0) {
        // Return squared distance between point and start-point of line segment
        glm::vec2 startPointToPoint = point - linePoint1;
        return glm::dot(startPointToPoint, startPointToPoint);
    }

    glm::vec2 distanceVector = toPoint - (lineDirection * factor);
    return glm::dot(distanceVector, distanceVector);
}

glm::vec2 billiard::physics::startVelocity(const glm::vec2& targetVelocity, float distance) {

    glm::vec2 zero{0, 0};
    assert(targetVelocity != zero);
    glm::vec2 normalized = glm::normalize(targetVelocity);
    float squaredLength = glm::dot(targetVelocity, targetVelocity);
    float length = sqrtf(squaredLength + 2 * gravitationalAcceleration * frictionCoefficient * distance);
    return length * normalized;
}

glm::vec2 billiard::physics::elasticCollisionTargetPosition(const glm::vec2& targetBallPosition, const glm::vec2& targetDirection, const float ballRadius) {
    glm::vec2 zero{0, 0};
    assert(targetDirection != zero);
    return targetBallPosition - 2 * ballRadius * glm::normalize(targetDirection);
}

glm::vec2 billiard::physics::elasticCollisionReverse(const glm::vec2& targetVelocity, const glm::vec2& originVelocity) {
    glm::vec2 zero{0, 0};
    assert(originVelocity != zero);
    glm::vec2 originVelocityNormalized = glm::normalize(originVelocity);
    float denom = glm::dot(originVelocityNormalized, targetVelocity);
    assert(denom != 0.0f);
    return (glm::dot(targetVelocity, targetVelocity) / denom) * originVelocityNormalized * (1 + energyLossByBall);
}

std::pair<glm::vec2, glm::vec2> billiard::physics::elasticCollision(const glm::vec2& position1, const glm::vec2& velocity1, const glm::vec2& position2, const glm::vec2& velocity2) {

    glm::vec2 distanceVector = position2 - position1;
    glm::vec2 zero{0, 0};
    assert(distanceVector != zero);
    glm::vec2 z = glm::normalize(distanceVector);

    auto velocityWithEnergyLoss1 = velocity1 / (1 + energyLossByBall);
    auto velocityWithEnergyLoss2 = velocity2 / (1 + energyLossByBall);

    glm::vec2 v1z = glm::dot(velocityWithEnergyLoss1, z) * z;
    glm::vec2 v2z = glm::dot(velocityWithEnergyLoss2, z) * z;
    glm::vec2 v1t = velocityWithEnergyLoss1 - v1z;
    glm::vec2 v2t = velocityWithEnergyLoss2 - v2z;

    glm::vec2 newVelocity1 = (v2z + v1t);
    glm::vec2 newVelocity2 = (v1z + v2t);

    return std::make_pair(newVelocity1, newVelocity2);
}

glm::vec2 billiard::physics::railCollision(const glm::vec2& velocity, const glm::vec2& normal) {
    DEBUG("[railCollision]: "
              << "velocity=" << velocity << " "
              << "normal=" << normal << " "
              << std::endl);

    return glm::reflect(velocity, normal) / (1 + energyLossByRail);
}

glm::vec2 billiard::physics::perp(glm::vec2 vector) {
    return glm::vec2{-vector.y, vector.x};
}

std::optional<float> billiard::physics::timeToCollisionWithRail(const glm::vec2& ballPosition,
                                                                const glm::vec2& ballVelocity,
                                                                const glm::vec2& acceleration,
                                                                const glm::vec2& railPoint1,
                                                                const glm::vec2& railPoint2,
                                                                const glm::vec2& shiftedRailPoint1,
                                                                const glm::vec2& shiftedRailPoint2) {

    auto shiftedSegmentIntersection = billiard::physics::intersection::halfLineIntersectsLineSegment(ballPosition, ballVelocity, shiftedRailPoint1, shiftedRailPoint2);
    float lambda = 0.0f;
    if (shiftedSegmentIntersection) {
        lambda = shiftedSegmentIntersection->first;
    } else {
        auto segmentIntersection = billiard::physics::intersection::halfLineIntersectsLineSegment(ballPosition, ballVelocity, railPoint1, railPoint2);
        if (segmentIntersection) {
            auto intersection = billiard::physics::intersection::lineIntersectsLine(ballPosition, ballVelocity, shiftedRailPoint1, shiftedRailPoint2 - shiftedRailPoint1);
            assert(intersection);
            lambda = intersection->first;
        } else {
            return std::nullopt;
        }
    }

    if (lambda < 0.0f) {
        DEBUG("[timeToCollisionWithRail]: no rail collision ... "
                      << "segment intersection lambda=" << lambda
                      << std::endl);
        return std::nullopt;
    }

    float distance = glm::length(lambda * ballVelocity);

    if (distance <= 0.00000000001f) { // TODO: Epsilon definieren
        DEBUG("[timeToCollisionWithRail]: no rail collision ... "
                      << "lambda=" << lambda
                      << "distance=" << distance
                      << std::endl);
        return std::nullopt;
    }

    // Kugel A rollt zu dieser Position und kollidiert mit der Bande.
    glm::vec2 ballTargetPoint = ballPosition + lambda * ballVelocity; // TODO: remove

    //float distance = glm::distance(ballPosition, ballTargetPoint);
    float velocity = glm::length(ballVelocity);
    float a = -glm::length(acceleration);

    auto result = billiard::physics::intersection::solveQuadraticFormula(0.5f * a, velocity, -distance);
    if (result.empty()) {
        DEBUG("[timeToCollisionWithRail]: "
                      << "no time of collision"
                      << std::endl);
        return std::nullopt;
    } else {
        // Values in Result of solveQuadraticFormula are sorted
        float time = result[0];

        DEBUG("[timeToCollisionWithRail]: "
                      << "lambda1=" << lambda << " "
                      << "ballTargetPoint=" << ballTargetPoint << " "
                      << "distance=" << distance << " "
                      << "velocity=" << velocity << " "
                      << "acceleration=" << acceleration << " "
                      << "a=" << a << " "
                      << "time=" << time << " "
                      << std::endl);

        return std::make_optional(time);
    }

    DEBUG("[timeToCollisionWithRail]: "
              << "no intersection point found"
              << std::endl);
    return std::nullopt;
}

float billiard::physics::timeToStop(const glm::vec2& acceleration, const glm::vec2& velocity) {
    float timeX = (-velocity.x / acceleration.x);
    float timeY = (-velocity.y / acceleration.y);

    DEBUG("[timeToStop]: "
              << "acceleration=" << acceleration << " "
              << "velocity=" << velocity << " "
              << "timeX=" << timeX << " "
              << "timeY=" << timeY << " "
              << std::endl);

    return std::max(timeX, timeY);
}

float billiard::physics::accelerationLength() {
    return -gravitationalAcceleration * frictionCoefficient;
}

glm::vec2 billiard::physics::acceleration(const glm::vec2& velocity, const float accelerationLength) {
    return accelerationByNormed(normalize(velocity), accelerationLength);
}

glm::vec2 billiard::physics::accelerationByNormed(const glm::vec2& velocity, const float accelerationLength) {
    return accelerationLength * velocity;
}

glm::vec2 billiard::physics::accelerate(const glm::vec2& acceleration, const glm::vec2& currentVelocity, float deltaTime) {
    return acceleration * deltaTime + currentVelocity;
}

glm::vec2 billiard::physics::position(const glm::vec2& acceleration, const glm::vec2& velocity, float deltaTime, const glm::vec2& position) {
    return (0.5f * acceleration * deltaTime * deltaTime) + (velocity * deltaTime) + position;
}

std::optional<float> billiard::physics::timeToCollision(const glm::vec2& acceleration1, const glm::vec2& velocity1,
                                                  const glm::vec2& position1, const glm::vec2& acceleration2,
                                                  const glm::vec2& velocity2, const glm::vec2& position2, float diameter,
                                                  float radius) {

    static glm::vec2 zero {0, 0};
    assert(velocity1 != zero);

    auto velocity2SquaredLength = glm::dot(velocity2, velocity2);
    if (velocity2SquaredLength > 0) {
        // Both balls are moving
        // TODO: document

        auto velocityIntersection = intersection::halfLineIntersectsHalfLine(position1, velocity1, position2, velocity2);
        if (!velocityIntersection) {

            glm::vec2 from1To2 = position2 - position1;
            glm::vec2 velocity1Normalised = glm::normalize(velocity1);
            glm::vec2 velocity2Normalised = glm::normalize(velocity2);
            glm::vec2 offset = from1To2 - velocity1Normalised * glm::dot(from1To2, velocity1Normalised);
            glm::vec2 backwardOffset1 = velocity1Normalised * -radius;
            glm::vec2 backwardOffset2 = velocity2Normalised * -radius;

            auto shiftedIntersection = intersection::halfLineIntersectsHalfLine(position1 + offset + backwardOffset1,
                                                                                velocity1, position2 + backwardOffset2,
                                                                                velocity2);
            if (!shiftedIntersection) {
                return std::nullopt;
            }
        }
    } else {
        // Second ball is stationary
        auto distance = pointToHalfLineSquaredDistance(position1, velocity1, position2);
        if (distance > diameter * diameter) {
            return std::nullopt;
        }
    }
    // TODO: Evtl. weitere möglich Checks.

    auto diameterInMeters = diameter / 1000.0f;
    auto deltaAcceleration = (acceleration1 - acceleration2) / 1000.0f;
    auto deltaVelocity = (velocity1 - velocity2) / 1000.0f;
    auto deltaPosition = (position1 - position2) / 1000.0f;
    float a = floorf(0.25f * glm::dot(deltaAcceleration, deltaAcceleration) * 1000000) / 1000000;
    float b = floorf(glm::dot(deltaAcceleration, deltaVelocity) * 1000000) / 1000000;
    float c = floorf((glm::dot(deltaAcceleration, deltaPosition) + glm::dot(deltaVelocity, deltaVelocity)) * 1000000) / 1000000;
    float d = floorf(2 * (glm::dot(deltaVelocity, deltaPosition)) * 1000000) / 1000000;
    float e = floorf((glm::dot(deltaPosition, deltaPosition) - (diameterInMeters * diameterInMeters)) * 1000000) / 1000000;

    auto solutions = intersection::solveQuarticFormula(a, b, c, d, e);
    auto collisionTimes = intersection::realRoot(solutions);

    if (collisionTimes.empty()) {
        DEBUG("[timeToCollision]: "
                      << "acceleration1=" << acceleration1 << " "
                      << "acceleration2=" << acceleration2 << " "
                      << "velocity1=" << velocity1 << " "
                      << "velocity2=" << velocity2 << " "
                      << "position1=" << position1 << " "
                      << "position2=" << position2 << " "
                      << "diameter=" << diameter << " "
                      << "a=" << a << " "
                      << "b=" << b << " "
                      << "c=" << c << " "
                      << "d=" << d << " "
                      << "e=" << e << " "
                      << "firstCollision=None"
                      << std::endl);

        return std::nullopt;
    }
    float firstCollision = collisionTimes.at(0);

    DEBUG("[timeToCollision]: "
              << "acceleration1=" << acceleration1 << " "
              << "acceleration2=" << acceleration2 << " "
              << "velocity1=" << velocity1 << " "
              << "velocity2=" << velocity2 << " "
              << "position1=" << position1 << " "
              << "position2=" << position2 << " "
              << "diameter=" << diameter << " "
              << "a=" << a << " "
              << "b=" << b << " "
              << "c=" << c << " "
              << "d=" << d << " "
              << "e=" << e << " "
              << "collisionTimes=" << collisionTimes << " "
              << "firstCollision=" << firstCollision << " "
              << std::endl);

    return firstCollision < 0.0f ? std::nullopt : std::make_optional<float>(firstCollision);
}

std::optional<float> billiard::physics::timeToTarget(const glm::vec2& targetPosition,
                                  const glm::vec2& position,
                                  const glm::vec2& normalizedVelocity,
                                  const glm::vec2& velocity,
                                  const glm::vec2& acceleration,
                                  float radius) {

    glm::vec2 zero{0, 0};
    assert(velocity != zero);

    auto velocityFactor = intersection::lineIntersectsCircle(targetPosition, radius, position, normalizedVelocity);

    if (velocityFactor.empty()) {
        DEBUG("[timeToTarget] Velocity factors is empty "
                  << "targetPosition=" << targetPosition << " "
                  << "position=" << position << " "
                  << "normalizedVelocity=" << normalizedVelocity << " "
                  << "velocity=" << velocity << " "
                  << "acceleration=" << acceleration << " "
                  << "radius=" << radius << " "
                  << std::endl);
        return std::nullopt;
    }

    float l = velocityFactor.at(0);
    if (l < 0.0f) { // Loch ist entgegengesetzt zu Kugel-Velocity
        DEBUG("[timeToTarget] Target on opposite direction "
                  << "targetPosition=" << targetPosition << " "
                  << "position=" << position << " "
                  << "normalizedVelocity=" << normalizedVelocity << " "
                  << "velocity=" << velocity << " "
                  << "acceleration=" << acceleration << " "
                  << "radius=" << radius << " "
                  << std::endl);
        return std::nullopt;
    }

    auto distance = l * normalizedVelocity;
    auto a = -0.5f * glm::length(acceleration);
    auto b = glm::length(velocity);
    auto c = - glm::length(distance);
    auto times = intersection::solveQuadraticFormula(a, b, c);
    if (times.empty()) { // Kugel rollt vorher aus
        DEBUG("[timeToTarget] Ball stops before "
                  << "targetPosition=" << targetPosition << " "
                  << "position=" << position << " "
                  << "normalizedVelocity=" << normalizedVelocity << " "
                  << "velocity=" << velocity << " "
                  << "acceleration=" << acceleration << " "
                  << "radius=" << radius << " "
                  << std::endl);
        return std::nullopt;
    }

    float firstIntersection = times.at(0);

    DEBUG("[timeToTarget]: "
              << "a=" << a << " "
              << "b=" << b << " "
              << "c=" << c << " "
              << "l=" << l << " "
              << "distance=" << distance << " "
              << "targetPosition=" << targetPosition << " "
              << "position=" << position << " "
              << "normalizedVelocity=" << normalizedVelocity << " "
              << "velocity=" << velocity << " "
              << "acceleration=" << acceleration << " "
              << "radius=" << radius << " "
              << "firstIntersection=" << firstIntersection << " "
              << std::endl);

    return firstIntersection < 0.0f ? std::nullopt : std::make_optional<float>(firstIntersection);
}

struct complex_order
{
    inline bool operator() (const std::complex<double>& complexLeft, const std::complex<double>& complexRight) {
        if (complexLeft.real() != complexRight.real()) {
            return (complexLeft.real() < complexRight.real());
        }

        return complexLeft.imag() < complexRight.imag();
    }
};

std::vector<std::complex<double>> billiard::physics::intersection::solveQuarticFormula(double a, double b,
                                                                        double c, double d,
                                                                        double e) {
    double discriminant = (256*a*a*a*e*e*e) - (192*a*a*b*d*e*e) - (128*a*a*c*c*e*e) + (144*a*a*c*d*d*e)
            -(27*a*a*d*d*d*d) + (144*a*b*b*c*e*e) - (6*a*b*b*d*d*e) - (80*a*b*c*c*d*e) + (18*a*b*c*d*d*d)
            +(16*a*c*c*c*c*e) -(4*a*c*c*c*d*d) - (27*b*b*b*b*e*e) + (18*b*b*b*c*d*e)
            -(4*b*b*b*d*d*d) - (4*b*b*c*c*c*e) + (b*b*c*c*d*d);
    std::complex<double> p = ((8 * a * c) - (3 * b * b)) / (8 * a * a);
    std::complex<double> q = (std::pow(b, 3) - (4 * a * b * c) + (8 * a * a * d)) / (8 * std::pow(a, 3));
    std::complex<double> deltaZero = (c * c) - (3 * b * d) + (12 * a * e);
    std::complex<double> deltaOne = (2 * std::pow(c, 3)) - (9 * b * c * d) + (27 * b * b * e) + (27 * a * d * d) - (72 * a * c * e);

    std::complex<double> powerOfDeltaZero = (4.0 * std::pow(deltaZero, 3));
    std::complex<double> deltaOneSquared = (deltaOne * deltaOne);
    std::complex<double> QInnerSqrt = std::sqrt(deltaOneSquared - powerOfDeltaZero);
    std::complex<double> QInnerCbrt = (deltaOne + QInnerSqrt) / 2.0;
    std::complex<double> Q = std::pow(QInnerCbrt, (1.0/3.0));

    // Spezialfall 1 -> Diskriminante != 0 & DeltaZero = 0 & Q = 0
    if (Q == 0.0 && discriminant != 0 && deltaZero == 0.0) {
        QInnerCbrt = (deltaOne - QInnerSqrt) / 2.0;
        Q = std::pow(QInnerCbrt, (1.0/3.0));
    }

    // Spezialfall 2 -> S = 0
    std::complex<double> S = 0.5 * std::sqrt((-(2.0/3.0) * p) + ((1.0/(3.0 * a)) * (Q + (deltaZero / Q))));
    if (S == 0.0) {
        QInnerCbrt = (deltaOne - QInnerSqrt) / 2.0;
        Q = std::pow(QInnerCbrt, (1.0/3.0));
        S = 0.5 * std::sqrt((-(2.0/3.0) * p) + ((1.0/(3.0 * a)) * (Q + (deltaZero / Q))));
    }

    std::complex<double> discriminantShared1 = (-4.0 * S * S) - (2.0 * p);
    std::complex<double> discriminantShared2 = q / S;

    std::complex<double> discriminant1 = discriminantShared1 + discriminantShared2;
    std::complex<double> discriminant2 = discriminantShared1 - discriminantShared2;

    std::complex<double> firstPartShared = (-b / (4 * a));
    std::complex<double> firstPartOfDiscriminant1 = firstPartShared - S;
    std::complex<double> firstPartOfDiscriminant2 = firstPartShared + S;

    auto solutions = solveQuartic(firstPartOfDiscriminant1, discriminant1);
    auto solutions2 = solveQuartic(firstPartOfDiscriminant2, discriminant2);

    solutions.insert(solutions.end(), solutions2.begin(), solutions2.end());
    std::sort(solutions.begin(), solutions.end(), complex_order());

    DEBUG("[solveQuarticFormula] Complex soutions -> "
              << "x0=" << solutions.at(0) << " "
              << "x1=" << solutions.at(1) << " "
              << "x2=" << solutions.at(2) << " "
              << "x3=" << solutions.at(3) << " "
              << "discriminant=" << discriminant
              << std::endl);

    return solutions;
}

std::vector<std::complex<double>> billiard::physics::intersection::solveQuartic(std::complex<double> firstPart, std::complex<double> discriminant) {
    std::complex<double> sqrt = 0.5 * std::sqrt(discriminant);
    auto x0 = firstPart + sqrt;
    auto x1 = firstPart - sqrt;

    DEBUG("[solveQuartic]: "
              << "firstPart=" << firstPart << " "
              << "discriminant=" << discriminant << " "
              << "sqrt=" << sqrt << " "
              << "x0=" << x0 << " "
              << "x1=" << x1 << " "
              << std::endl);

    return std::vector<std::complex<double>> {x0, x1};
}

std::vector<float> billiard::physics::intersection::realRoot(const std::vector<std::complex<double>>& roots) {
    std::vector<float> solutions;
    for (auto& solution : roots) {
        if (std::abs(solution.imag()) < 0.001f) { // TODO: Epsilon definieren
            solutions.push_back(static_cast<float>(solution.real()));
        }
    }
    return solutions;
}

std::vector<float> billiard::physics::intersection::solveQuadraticFormula(float a, float b, float c) {
    float discriminant = b*b - 4*a*c;

    if (discriminant < 0) {
        return {};
    }

    if (discriminant == 0) {
        return { -0.5f * b / a };
    }

    // see "instability of the quadratic equation" for why this does not look like the "regular" quadratic equation
    float q = (b > 0) ?
              -0.5f * (b + sqrt(discriminant)) :
              -0.5f * (b - sqrt(discriminant));
    float x0 = q / a;
    float x1 = c / q;

    // Sort values, so that minimal value is first
    return x0 <= x1 ? std::vector<float> {x0, x1} : std::vector<float> {x1, x0};
}

billiard::physics::intersection::LineIntersection billiard::physics::intersection::lineIntersectsCircle(const glm::vec2& center,
                                                                          float radius,
                                                                          const glm::vec2& pointOnLine,
                                                                          const glm::vec2& lineDirection) {

    // circle equation is S: |X - C|^2 = r^2
    //                    S: (x - Cx)^2 + (y - Cy)^2 = r^2
    // Parametric equation of a line is: d = p + t * v
    //                                   dx = Px + t * Vx
    //                                   dy = Py + t * Vy
    // Insert equation of line into equation of circle and solve for t:
    // (Px + t * Vx - Cx)^2 + (Py + t * Vy - Cy)^2 = r^2
    // |P + t*V - C|^2 = r^2
    // |t*V + P - C|^2 = r^2
    // |t*V + (P-C)|^2 = r^2
    // (t*V + (P-C)) * (t*V + (P-C)) = r^2
    // t*t*(V*V) + 2t*(V*(P-C)) + (P-C)*(P-C) = r^2
    // t*t*(V*V) + 2t*(V*(P-C)) + (P-C)*(P-C) - r^2 = 0
    // is a quadratic formula of the form:
    // a^2 * t + b * t + c = 0
    // where:
    // a = V*V = |V|^2 = 1 (since V must be normalized)
    // b = 2*V*(P-C)
    // c = (P-C)*(P-C) - r^2 = |(P-C)|^2 - r^2
    // the general solution of the quadratic formula is:
    // t1 = (-b + sqrt(b^2 - 4ac)) / 2a
    // t2 = (-b - sqrt(b^2 - 4ac)) / 2a
    // where b^2 - 4ac is the discriminant
    // There are three cases to consider:
    // - discriminant > 0: two solutions (ray pierces circle)
    //   t1 = (-b + sqrt(b^2 - 4ac)) / 2a
    //   t2 = (-b - sqrt(b^2 - 4ac)) / 2a
    // - discriminant = 0: one solution (ray grazes circle)
    //   t = -b / 2a
    // - discriminant < 0: no solution (ray neither intersects nor grazes circle)

    glm::vec2 delta = pointOnLine - center;
    float a = 1;
    float b = 2 * glm::dot(lineDirection, delta);
    float c = glm::dot(delta, delta) - radius * radius;
    return solveQuadraticFormula(a, b, c);
}

std::vector<float> billiard::physics::nonNegative(const std::vector<float>&& values) {
    std::vector<float> filtered;
    for (auto value : values) {
        if (value >= 0.0f) {
            filtered.push_back(value);
        }
    }
    return filtered;
}

glm::vec2 billiard::physics::normalize(const glm::vec2& vector) {
    if (vector == glm::vec2{0, 0}) {
        return glm::vec2{0, 0};
    }
    return glm::normalize(vector);
}

billiard::physics::intersection::RayIntersection billiard::physics::intersection::rayIntersectsCircle(const glm::vec2& center,
                                                                        float radius,
                                                                        const glm::vec2& rayOrigin,
                                                                        const glm::vec2& rayDirection) {
    return nonNegative(lineIntersectsCircle(center, radius, rayOrigin, rayDirection));
}

billiard::physics::intersection::LineLineIntersection
billiard::physics::intersection::lineIntersectsLine(const glm::vec2& linePoint1, const glm::vec2& lineDirection1,
                                     const glm::vec2& linePoint2, const glm::vec2& lineDirection2) {

    static glm::vec2 zero {0, 0};
    assert(lineDirection1 != zero);
    assert(lineDirection2 != zero);
    // Assuming the following parametric equations of the 2D lines are given:
    // L1(t) = P + t * D
    // L2(u) = Q + u * V
    // P: a point on line 1
    // D: the direction vector of line 1
    // Q: a point on line 2
    // V: the direction vector of line 2
    //
    // Then the intersection can be found by setting both equations equal:
    // L1(t) = L2(t)
    // P + t * D = Q + u * V
    //
    // Write equations for both components:
    // Px + t * Dx = Qx + u * Vx
    // Py + t * Dy = Qy + u * Vy
    //
    // Isolate one unknown:
    // Px + t * Dx - Qx = u * Vx
    // Py + t * Dy - Qy = u * Vy
    //
    // (Px + t * Dx - Qx) / Vx = u
    // (Py + t * Dy - Qy) / Vy = u
    //
    // Set both equations equal to get rid of the variable u:
    // (Px + t * Dx - Qx) / Vx = (Py + t * Dy - Qy) / Vy
    //
    // Rearrange to isolate t:
    // (Px + t * Dx - Qx) * Vy = (Py + t * Dy - Qy) * Vx
    // Px*Vy + t*Dx*Vy - Qx*Vy = Py*Vx + t*Dy*Vx - Qy*Vx
    // t*Dx*Vy - t*Dy*Vx = Py*Vx - Px*Vy - Qy*Vx + Qx*Vy
    // t*(Dx*Vy - Dy*Vx) = Py*Vx - Px*Vy - Qy*Vx + Qx*Vy
    // t = (Py*Vx - Px*Vy - Qy*Vx + Qx*Vy) / (Dx*Vy - Dy*Vx)
    // t = (Vx*Py - Vy*Px + Qx*Vy - Qy*Vx) / (Dx*Vy - Dy*Vx)
    //
    // This formula can be rewritten using the determinant:
    //            Vx  Px            Qx  Vx
    //       det( Vy  Py )  +  det( Qy  Vy )
    // t =   -------------------------------
    //                    Dx  Vx
    //               det( Dy  Vy )
    //                                                      Dx  Vx
    // If the lower determinant is equal to zero, i.e. det( Dy  Vy ) = 0
    // then there is no intersection between the two lines
    //
    // Now t can be inserted into an equation derived previously to find the variable u:
    // (Px + t * Dx - Qx) / Vx = u
    // (Py + t * Dy - Qy) / Vy = u

    const auto& P = linePoint1;
    const auto& D = lineDirection1;
    const auto& Q = linePoint2;
    const auto& V = lineDirection2;

    float detDV = D.x*V.y - D.y*V.x;
    if (detDV == 0.0f) {
        return std::nullopt;
    }

    float detVP = V.x*P.y - V.y*P.x;
    float detQV = Q.x*V.y - Q.y*V.x;
    float t = (detVP + detQV) / detDV;

    float u = V.x != 0.0f ? (P.x + t * D.x - Q.x) / V.x : (P.y + t * D.y - Q.y) / V.y;

    return std::make_optional(std::make_pair(t, u));
}

billiard::physics::intersection::LineLineIntersection
billiard::physics::intersection::halfLineIntersectsLineSegment(const glm::vec2 &halfLinePoint, const glm::vec2 &halfLineDirection,
                                                const glm::vec2 &lineSegmentStartPoint,
                                                const glm::vec2 &lineSegmentEndPoint) {

    billiard::physics::intersection::LineLineIntersection intersection = lineIntersectsLine(halfLinePoint,
                                                                             halfLineDirection,
                                                                             lineSegmentStartPoint,
                                                                             lineSegmentEndPoint - lineSegmentStartPoint);

    if (!intersection.has_value()) {
        return intersection;
    }

    float t = intersection->first;  // Factor for half-line
    float u = intersection->second; // Factor for line segment

    // Make sure that point of intersection lies on half-line
    if (t < 0.0f) {
        return std::nullopt;
    }

    // Make sure that point of intersection lies on the line segment
    if (u < 0.0f || u > 1.0f) {
        return std::nullopt;
    }

    return std::make_optional(std::make_pair(t, u));
}

billiard::physics::intersection::LineLineIntersection
billiard::physics::intersection::halfLineIntersectsHalfLine(const glm::vec2& halfLinePoint1,
                           const glm::vec2& halfLineDirection1,
                           const glm::vec2& halfLinePoint2,
                           const glm::vec2& halfLineDirection2) {
    billiard::physics::intersection::LineLineIntersection intersection = lineIntersectsLine(halfLinePoint1,
                                                                                            halfLineDirection1,
                                                                                            halfLinePoint2,
                                                                                            halfLineDirection2);

    if (!intersection.has_value()) {
        return intersection;
    }

    float t = intersection->first;  // Factor for first half-line
    float u = intersection->second; // Factor for second half-line

    // Make sure that point of intersection lies on first half-line
    if (t < 0.0f) {
        return std::nullopt;
    }

    // Make sure that point of intersection lies on second half-line
    if (u < 0.0f) {
        return std::nullopt;
    }

    return std::make_optional(std::make_pair(t, u));
}
