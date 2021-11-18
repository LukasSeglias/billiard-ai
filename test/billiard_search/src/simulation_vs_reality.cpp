#include <gtest/gtest.h>
#include <billiard_search/billiard_search.hpp>
#include <billiard_snooker/billiard_snooker.hpp>
#include <billiard_physics/billiard_physics.hpp>
#include <fstream>
#include "util.hpp"

#define SOLUTIONS 10

using billiard::search::node::System;
using billiard::search::node::NodeType;
using billiard::search::state::BallState;
using billiard::search::node::Layer;

std::optional<billiard::search::node::Node> getNodeOfBall(const billiard::search::node::Layer& layer,
                                                          const std::string& ballId,
                                                          NodeType nodeType) {

    for (auto& node : layer._nodes) {
        if (node.first == ballId && node.second._type == nodeType) {
            return node.second;
        }
    }
    return std::nullopt;
}

float checkPosition(const BallState& ballState, const glm::vec2& position) {
    return glm::length(position - ballState._position);
}

float checkTime(const Layer& layer, float time) {
    return time - layer._time;
}

float checkDirection(const glm::vec2& actual, const glm::vec2& expected) {
    float angleRadians = 1.0f - glm::dot(glm::normalize(actual), glm::normalize(expected));
    // PI = 180 degrees
    return 180.0f / CV_PI * angleRadians;
}

glm::vec2 calculateInitialVelocity(const glm::vec2& traveledDistance, int frameDuration) {
    float duration = ((float) frameDuration) / 30.0f; // assuming 30 fps

    const float g = billiard::physics::gravitationalAcceleration; // mm/s^2
    const float mu_r = billiard::physics::frictionCoefficient;
    const float mu_g = billiard::physics::slideFrictionCoefficient;
    float s = glm::length(traveledDistance);

    float a = (-mu_r - 5.0f * mu_g)/(49.0f/2.0f * g * mu_g * mu_g) + 12.0f/(49.0f * g * mu_g);
    float b = (mu_r * duration)/(7.0f/2.0f * mu_g) + (5.0f * duration)/7.0f;
    float c = -0.5f * g * mu_r * duration * duration - s;

    auto result = billiard::physics::nonNegative(billiard::physics::intersection::solveQuadraticFormula(a, b, c));
    if (result.empty()) {
        return glm::vec2 {0, 0};
    }
    float v0 = result[0];

    glm::vec2 velocity = v0 * glm::normalize(traveledDistance);

    return velocity;
}

TEST(SimulationVsReality, video_1_0233_0240) {

    std::string name = "video_1_0233_0240";
    /*
    Weak impact, WHITE collides with RED, RED goes into pocket, WHITE stops.

    WHITE, WHITE, -566.395, 162.876
    RED, RED, -147.313, 307.532
    V, 554.316, 147.92

    WHITE from start to collision:
    WHITE at start: pixel=(290, 265) model=(-566.395, 162.876) frame=6
    WHITE at collision: pixel=(521, 203) model=(-181.055, 265.705) frame=29
    -> frames=23 duration=0.766667 vector=(385.34, 102.828) s=398.824 v0=573.713 velocity=(554.316, 147.92)

    RED from collision to potting:
    RED at collision: pixel=(541, 178) model=(-147.313, 307.532) frame=29
    RED at pocket: pixel=(617, 88) model=(-19.0418, 458.224) frame=62
    -> frames=33 duration=1.1 vector=(128.271, 150.693) s=197.893 v0=256.675 velocity=(166.372, 195.455)

    WHITE from collision to in rest:
    WHITE at collision: pixel=(522, 203) model=(-179.382, 265.702) frame=29
    WHITE in rest: pixel=(710, 279) model=(136.109, 137.307) frame=93
    -> frames=64 duration=2.13333 vector=(315.491, -128.395) s=340.617 v0=308.556 velocity=(285.795, -116.31)
    */
    const glm::vec2& whiteStart = glm::vec2{-566.395, 162.876};
    const glm::vec2& redStart = glm::vec2 {-145.628, 309.205};
    const glm::vec2& whiteCollision = glm::vec2{-181.055, 265.705};
    const glm::vec2& redCollision = glm::vec2 {-145.628, 309.205};
    const glm::vec2& whiteInRest = glm::vec2 {136.109, 137.307};
    const glm::vec2& whiteDirectionAfterCollision = whiteInRest - whiteCollision;
    const float collisionTime = 0.766667f;
    const float redPottedTime = 1.1f + collisionTime; // since collision
    const float whiteInRestTime = 2.1333f + collisionTime; // since beginning
//    glm::vec2 velocity {554.316, 147.92};
    glm::vec2 velocity = calculateInitialVelocity(whiteCollision - whiteStart, 23);

    std::string white = "WHITE-1";
    std::string red = "RED-1";
    billiard::search::State state({
        billiard::search::Ball { whiteStart, "WHITE", white},
        billiard::search::Ball { redStart, "RED", red},
    });

    billiard::search::Configuration config = loadConfig("./resources/configuration.json");

    std::optional<System> systemOpt = billiard::search::simulate(state, velocity, config);
    if (!systemOpt) {
        std::cout << "No result" << std::endl;
        return;
    }
    System system = systemOpt.value();

    std::cout << "Layers: " << std::to_string(system._layers.size()) << std::endl;
    EXPECT_EQ(system._layers.size(), 6);

    Layer& startLayer = system._layers[0];
    auto whiteMovingNode = getNodeOfBall(startLayer, white, NodeType::BALL_SHOT)->toBallShot();
    float whiteStartPositionError = checkPosition(whiteMovingNode->_ball, whiteStart);
    EXPECT_FLOAT_EQ(whiteStartPositionError, 0.0f);

    auto redInRestNode = getNodeOfBall(startLayer, red, NodeType::BALL_IN_REST)->toInRest();
    float redStartPositionError = checkPosition(redInRestNode->_ball, redStart);
    EXPECT_FLOAT_EQ(redStartPositionError, 0.0f);

    Layer& whiteRollingLayer = system._layers[1];
    auto whiteRollingNode = getNodeOfBall(whiteRollingLayer, white, NodeType::BALL_MOVING)->toBallMoving();
    EXPECT_TRUE(whiteRollingNode);
    EXPECT_EQ(whiteRollingNode->_after._isRolling, true);

    Layer& collisionLayer = system._layers[2];
    auto whiteCollisionNode = getNodeOfBall(collisionLayer, white, NodeType::BALL_COLLISION)->toBallCollision();
    EXPECT_TRUE(whiteCollisionNode);
    float whiteCollisionPositionError = checkPosition(whiteCollisionNode->_after, whiteCollision);

    auto redCollisionNode = getNodeOfBall(collisionLayer, red, NodeType::BALL_COLLISION)->toBallCollision();
    EXPECT_TRUE(redCollisionNode);
    float redCollisionPositionError = checkPosition(redCollisionNode->_after, redCollision);

    Layer& redRollingLayer = system._layers[3];
    auto redRollingNode = getNodeOfBall(redRollingLayer, red, NodeType::BALL_MOVING)->toBallMoving();
    EXPECT_TRUE(redRollingNode);
    EXPECT_EQ(redRollingNode->_after._isRolling, true);

    Layer& redPottedLayer = system._layers[4];
    auto redPottedNode = getNodeOfBall(redPottedLayer, red, NodeType::BALL_POTTING)->toPotted();
    EXPECT_TRUE(redPottedNode);
    float redPottedTimeError = checkTime(redPottedLayer, redPottedTime);

    Layer& whiteInRestLayer = system._layers[5];
    auto whiteInRestNode = getNodeOfBall(whiteInRestLayer, white, NodeType::BALL_IN_REST)->toInRest();
    EXPECT_TRUE(whiteInRestNode);
    float whiteInRestPositionError = checkPosition(whiteInRestNode->_ball, whiteInRest);
    float whiteInRestTimeError = checkTime(whiteInRestLayer, whiteInRestTime);
    float whiteInRestDirectionError = checkDirection(whiteInRestNode->_ball._position - whiteCollisionNode->_after._position, whiteDirectionAfterCollision);

    std::cout << "Test " << name << " "
//        << "start pos ["
//        << "white=" << std::to_string(whiteStartPositionError) << " "
//        << "red=" << std::to_string(redStartPositionError)
//        << "]" << " "
//        << std::endl
        << "collision pos ["
        << "white=" << std::to_string(whiteCollisionPositionError) << " "
        << "red=" << std::to_string(redCollisionPositionError)
        << "]" << " "
        << std::endl
        << "red potted ["
        << "time=" << std::to_string(redPottedTimeError) << " "
        << "]" << " "
        << std::endl
        << "white in rest ["
        << "pos=" << std::to_string(whiteInRestPositionError) << " "
        << "time=" << std::to_string(whiteInRestTimeError) << " "
        << "dir=" << std::to_string(whiteInRestDirectionError) << " "
        << "]" << " "
        << std::endl;
}

TEST(SimulationVsReality, video_9_0056_0101) {

    // NOTE: die weisse kugel scheint nach der Kollision mit der roten noch spin zu haben. Dieser Test ist demnach nicht zwingend verlässlich.
    std::string name = "video_9_0056_0101";
    /*
    Strong impact, WHITE collides with RED, RED goes into pocket, WHITE collides with rail and stops.

    WHITE, WHITE, -569.812, -164.689
    RED, RED, 880.914, 416.95
    V, 1885.89, 740.483

    WHITE from start to collision:
    WHITE at start: pixel=(290, 461) model=(-569.812, -164.689) frame=11
    WHITE at collision: pixel=(1121, 131) model=(837.855, 388.021) frame=34
    -> frames=23 duration=0.766667 vector=(1407.67, 552.711) s=1512.29 v0=2026.06 velocity=(1885.89, 740.483)

    RED from collision to pocket:
    RED at collision: pixel=(1146, 114) model=(880.914, 416.95) frame=34
    RED at pocket: pixel=(1186, 86) model=(949.879, 464.646) frame=36
    -> frames=2 duration=0.0666667 vector=(68.9648, 47.6955) s=83.8511 v0=1262.42 velocity=(1038.3, 718.079)

    WHITE from ball collision to rail collision
    WHITE at ball collision: pixel=(1120, 132) model=(836.135, 386.321) frame=35
    WHITE at rail collision: pixel=(1167, 151) model=(917.351, 354.115) frame=41
    -> frames=6 duration=0.2 vector=(81.2167, -32.2059) s=87.3691 v0=450.804 velocity=(419.059, -166.175)

    WHITE from rail collision to in rest:
    WHITE at rail collision: pixel=(1167, 150) model=(917.347, 355.816) frame=41
    WHITE in rest: pixel=(1030, 165) model=(681.38, 330.207) frame=94
    -> frames=53 duration=1.76667 vector=(-235.967, -25.6091) s=237.353 v0=257.652 velocity=(-256.148, -27.7993)
    */
    const glm::vec2& whiteStart = glm::vec2{-569.812, -164.689};
    const glm::vec2& redStart = glm::vec2 {880.914, 416.95};
    const glm::vec2& whiteCollision = glm::vec2{837.855, 388.021};
    const glm::vec2& whiteRailCollision = glm::vec2{917.351, 354.115};
    const glm::vec2& redCollision = glm::vec2 {880.914, 416.95};
    const glm::vec2& whiteInRest = glm::vec2 {681.38, 330.207};
    const glm::vec2& whiteDirectionAfterBallCollision = whiteRailCollision - whiteCollision;
    const glm::vec2& whiteDirectionAfterRailCollision = whiteInRest - whiteRailCollision;
    const float collisionTime = 0.766667f;
    const float redPottedTime = 0.0666667f + collisionTime; // since collision
    const float whiteRailCollisionTime = 0.2f + collisionTime; // since collision
    const float whiteInRestTime = 1.76667f + whiteRailCollisionTime; // since beginning
//    glm::vec2 velocity {1883.6, 738.188};
    glm::vec2 velocity = calculateInitialVelocity(whiteCollision - whiteStart, 23);

    std::string white = "WHITE-1";
    std::string red = "RED-1";
    billiard::search::State state({
          billiard::search::Ball { whiteStart, "WHITE", white},
          billiard::search::Ball { redStart, "RED", red},
    });

    billiard::search::Configuration config = loadConfig("./resources/configuration.json");

    std::optional<System> systemOpt = billiard::search::simulate(state, velocity, config);
    if (!systemOpt) {
        std::cout << "No result" << std::endl;
        return;
    }
    System system = systemOpt.value();

    std::cout << "Layers: " << std::to_string(system._layers.size()) << std::endl;
    EXPECT_EQ(system._layers.size(), 6);

    Layer& startLayer = system._layers[0];
    auto whiteMovingNode = getNodeOfBall(startLayer, white, NodeType::BALL_SHOT)->toBallShot();
    float whiteStartPositionError = checkPosition(whiteMovingNode->_ball, whiteStart);
    EXPECT_FLOAT_EQ(whiteStartPositionError, 0.0f);

    auto redInRestNode = getNodeOfBall(startLayer, red, NodeType::BALL_IN_REST)->toInRest();
    float redStartPositionError = checkPosition(redInRestNode->_ball, redStart);
    EXPECT_FLOAT_EQ(redStartPositionError, 0.0f);

    Layer& whiteRollingLayer = system._layers[1];
    auto whiteRollingNode = getNodeOfBall(whiteRollingLayer, white, NodeType::BALL_MOVING)->toBallMoving();
    EXPECT_TRUE(whiteRollingNode);
    EXPECT_EQ(whiteRollingNode->_after._isRolling, true);

    Layer& collisionLayer = system._layers[2];
    auto whiteCollisionNode = getNodeOfBall(collisionLayer, white, NodeType::BALL_COLLISION)->toBallCollision();
    EXPECT_TRUE(whiteCollisionNode);
    float whiteCollisionPositionError = checkPosition(whiteCollisionNode->_after, whiteCollision);

    auto redCollisionNode = getNodeOfBall(collisionLayer, red, NodeType::BALL_COLLISION)->toBallCollision();
    EXPECT_TRUE(redCollisionNode);
    float redCollisionPositionError = checkPosition(redCollisionNode->_after, redCollision);

    // RED probably does not start rolling

    Layer& redPottedLayer = system._layers[3];
    auto redPottedNode = getNodeOfBall(redPottedLayer, red, NodeType::BALL_POTTING)->toPotted();
    EXPECT_TRUE(redPottedNode);
    float redPottedTimeError = checkTime(redPottedLayer, redPottedTime);

    Layer& whiteRailCollisionLayer = system._layers[4];
    auto whiteRailCollisionNode = getNodeOfBall(whiteRailCollisionLayer, white, NodeType::BALL_RAIL_COLLISION)->toBallRailCollision();
    EXPECT_TRUE(whiteRailCollisionNode);
    float whiteRailCollisionPositionError = checkPosition(whiteRailCollisionNode->_after, whiteRailCollision);
    float whiteRailCollisionTimeError = checkTime(whiteRailCollisionLayer, whiteRailCollisionTime);
    float whiteRailCollisionDirectionError = checkDirection(whiteRailCollisionNode->_after._position - whiteCollisionNode->_after._position, whiteDirectionAfterBallCollision);

    Layer& whiteInRestLayer = system._layers[5];
    auto whiteInRestNode = getNodeOfBall(whiteInRestLayer, white, NodeType::BALL_IN_REST)->toInRest();
    EXPECT_TRUE(whiteInRestNode);
    float whiteInRestPositionError = checkPosition(whiteInRestNode->_ball, whiteInRest);
    float whiteInRestTimeError = checkTime(whiteInRestLayer, whiteInRestTime);
    float whiteInRestDirectionError = checkDirection(whiteInRestNode->_ball._position - whiteRailCollisionNode->_after._position, whiteDirectionAfterRailCollision);

    std::cout << "Test " << name << " "
//              << "start pos ["
//              << "white=" << std::to_string(whiteStartPositionError) << " "
//              << "red=" << std::to_string(redStartPositionError)
//              << "]" << " "
//              << std::endl
              << "collision pos ["
              << "white=" << std::to_string(whiteCollisionPositionError) << " "
              << "red=" << std::to_string(redCollisionPositionError)
              << "]" << " "
              << std::endl
              << "red potted ["
              << "time=" << std::to_string(redPottedTimeError) << " "
              << "]" << " "
              << std::endl
              << "white rail collision ["
              << "pos=" << std::to_string(whiteRailCollisionPositionError) << " "
              << "time=" << std::to_string(whiteRailCollisionTimeError) << " "
              << "dir=" << std::to_string(whiteRailCollisionDirectionError) << " "
              << "]" << " "
              << std::endl
              << "white in rest ["
              << "pos=" << std::to_string(whiteInRestPositionError) << " "
              << "time=" << std::to_string(whiteInRestTimeError) << " "
              << "dir=" << std::to_string(whiteInRestDirectionError) << " "
              << "]" << " "
              << std::endl;
}

TEST(SimulationVsReality, video_12_0130_0140) {

    // NOTE: ist dieser Testfall verlässlich? Die Kugel könnte womöglich side-spin bei der Kollision mit der Bande haben.
    std::string name = "video_12_0130_0140";
    /*

    WHITE, WHITE, 510.032, 76.0798
    RED, RED, 626.691, -152.605
    V, 773.745, -1932.59

    WHITE from start to collision:
    WHITE at start: pixel=(930, 315) model=(510.032, 76.0798) frame=233
    WHITE at collision: pixel=(975, 428) model=(587.147, -116.531) frame=236
    -> frames=3 duration=0.1 vector=(77.1151, -192.611) s=207.475 v0=2081.72 velocity=(773.745, -1932.59)

    RED from collision to pocket:
    RED at collision: pixel=(998, 449) model=(626.691, -152.605) frame=236
    RED in pocket: pixel=(1161, 615) model=(908.974, -439.608) frame=248
    -> frames=12 duration=0.4 vector=(282.284, -287.003) s=402.56 v0=1034.32 velocity=(725.285, -737.411)

    WHITE from collision to rail collision:
    WHITE at ball collision: pixel=(973, 429) model=(583.712, -118.215) frame=237
    WHITE at rail collision: pixel=(894, 620) model=(447.998, -443.152) frame=255
    -> frames=18 duration=0.6 vector=(-135.714, -324.937) s=352.14 v0=628.775 velocity=(-242.329, -580.202)

    WHITE from rail collision to in rest:
    WHITE at rail collision: pixel=(896, 622) model=(451.425, -446.609) frame=255
    WHITE in rest: pixel=(821, 532) model=(323.328, -291.762) frame=300
    -> frames=45 duration=1.5 vector=(-128.096, 154.847) s=200.964 v0=238.666 velocity=(-152.128, 183.898)
    */
    const glm::vec2& whiteStart = glm::vec2{510.032, 76.0798};
    const glm::vec2& redStart = glm::vec2 {626.691, -152.605};
    const glm::vec2& whiteCollision = glm::vec2{585.43, -118.225};
    const glm::vec2& whiteRailCollision = glm::vec2{447.998, -443.152};
    const glm::vec2& redCollision = glm::vec2 {626.691, -152.605};
    const glm::vec2& whiteInRest = glm::vec2 {323.328, -291.762};
    const glm::vec2& whiteDirectionAfterBallCollision = whiteRailCollision - whiteCollision;
    const glm::vec2& whiteDirectionAfterRailCollision = whiteInRest - whiteRailCollision;
    const float collisionTime = 0.1f;
    const float redPottedTime = 0.4f + collisionTime; // since collision
    const float whiteRailCollisionTime = 0.6f + collisionTime; // since collision
    const float whiteInRestTime = 1.5f + whiteRailCollisionTime; // since beginning
//    glm::vec2 velocity {790.738, -2000.58};
    glm::vec2 velocity = calculateInitialVelocity(whiteCollision - whiteStart, 3);

    std::string white = "WHITE-1";
    std::string red = "RED-1";
    billiard::search::State state({
                                          billiard::search::Ball { whiteStart, "WHITE", white},
                                          billiard::search::Ball { redStart, "RED", red},
                                  });

    billiard::search::Configuration config = loadConfig("./resources/configuration.json");

    std::optional<System> systemOpt = billiard::search::simulate(state, velocity, config);
    if (!systemOpt) {
        std::cout << "No result" << std::endl;
        return;
    }
    System system = systemOpt.value();

    std::cout << "Layers: " << std::to_string(system._layers.size()) << std::endl;
    EXPECT_EQ(system._layers.size(), 7);

    Layer& startLayer = system._layers[0];
    auto whiteMovingNode = getNodeOfBall(startLayer, white, NodeType::BALL_SHOT)->toBallShot();
    float whiteStartPositionError = checkPosition(whiteMovingNode->_ball, whiteStart);
    EXPECT_FLOAT_EQ(whiteStartPositionError, 0.0f);

    auto redInRestNode = getNodeOfBall(startLayer, red, NodeType::BALL_IN_REST)->toInRest();
    float redStartPositionError = checkPosition(redInRestNode->_ball, redStart);
    EXPECT_FLOAT_EQ(redStartPositionError, 0.0f);

    Layer& whiteRollingLayer = system._layers[1];
    auto whiteRollingNode = getNodeOfBall(whiteRollingLayer, white, NodeType::BALL_MOVING)->toBallMoving();
    EXPECT_TRUE(whiteRollingNode);
    EXPECT_EQ(whiteRollingNode->_after._isRolling, true);

    Layer& collisionLayer = system._layers[2];
    auto whiteCollisionNode = getNodeOfBall(collisionLayer, white, NodeType::BALL_COLLISION)->toBallCollision();
    EXPECT_TRUE(whiteCollisionNode);
    float whiteCollisionPositionError = checkPosition(whiteCollisionNode->_after, whiteCollision);

    auto redCollisionNode = getNodeOfBall(collisionLayer, red, NodeType::BALL_COLLISION)->toBallCollision();
    EXPECT_TRUE(redCollisionNode);
    float redCollisionPositionError = checkPosition(redCollisionNode->_after, redCollision);

    Layer& redRollingLayer = system._layers[3];
    auto redRollingNode = getNodeOfBall(redRollingLayer, red, NodeType::BALL_MOVING)->toBallMoving();
    EXPECT_TRUE(redRollingNode);
    EXPECT_EQ(redRollingNode->_after._isRolling, true);

    Layer& redPottedLayer = system._layers[4];
    auto redPottedNode = getNodeOfBall(redPottedLayer, red, NodeType::BALL_POTTING)->toPotted();
    EXPECT_TRUE(redPottedNode);
    float redPottedTimeError = checkTime(redPottedLayer, redPottedTime);

    Layer& whiteRailCollisionLayer = system._layers[5];
    auto whiteRailCollisionNode = getNodeOfBall(whiteRailCollisionLayer, white, NodeType::BALL_RAIL_COLLISION)->toBallRailCollision();
    EXPECT_TRUE(whiteRailCollisionNode);
    float whiteRailCollisionPositionError = checkPosition(whiteRailCollisionNode->_after, whiteRailCollision);
    float whiteRailCollisionTimeError = checkTime(whiteRailCollisionLayer, whiteRailCollisionTime);
    float whiteRailCollisionDirectionError = checkDirection(whiteRailCollisionNode->_after._position - whiteCollisionNode->_after._position, whiteDirectionAfterBallCollision);

    Layer& whiteInRestLayer = system._layers[6];
    auto whiteInRestNode = getNodeOfBall(whiteInRestLayer, white, NodeType::BALL_IN_REST)->toInRest();
    EXPECT_TRUE(whiteInRestNode);
    float whiteInRestPositionError = checkPosition(whiteInRestNode->_ball, whiteInRest);
    float whiteInRestTimeError = checkTime(whiteInRestLayer, whiteInRestTime);
    float whiteInRestDirectionError = checkDirection(whiteInRestNode->_ball._position - whiteRailCollisionNode->_after._position, whiteDirectionAfterRailCollision);

    std::cout << "Test " << name << " "
//              << "start pos ["
//              << "white=" << std::to_string(whiteStartPositionError) << " "
//              << "red=" << std::to_string(redStartPositionError)
//              << "]" << " "
//              << std::endl
              << "collision pos ["
              << "white=" << std::to_string(whiteCollisionPositionError) << " "
              << "red=" << std::to_string(redCollisionPositionError)
              << "]" << " "
              << std::endl
              << "red potted ["
              << "time=" << std::to_string(redPottedTimeError) << " "
              << "]" << " "
              << std::endl
              << "white rail collision ["
              << "pos=" << std::to_string(whiteRailCollisionPositionError) << " "
              << "time=" << std::to_string(whiteRailCollisionTimeError) << " "
              << "dir=" << std::to_string(whiteRailCollisionDirectionError) << " "
              << "]" << " "
              << std::endl
              << "white in rest ["
              << "pos=" << std::to_string(whiteInRestPositionError) << " "
              << "time=" << std::to_string(whiteInRestTimeError) << " "
              << "dir=" << std::to_string(whiteInRestDirectionError) << " "
              << "]" << " "
              << std::endl;
}

TEST(SimulationVsReality, video_12_0205_0208) {

    std::string name = "video_12_0205_0208";
    /*
    WHITE, WHITE, 504.88, 77.68
    RED, RED, 630.137, -152.789
    V, 144.223, -344.953

    WHITE from start to collision:
    WHITE at start: pixel=(927, 314) model=(504.88, 77.68) frame=22
    WHITE at collision: pixel=(974, 427) model=(585.423, -114.962) frame=41
    -> frames=19 duration=0.633333 vector=(80.5428, -192.642) s=208.802 v0=373.889 velocity=(144.223, -344.953)

    RED from collision to in rest:
    RED at collision: pixel=(1000, 449) model=(630.137, -152.789) frame=41
    RED in rest: pixel=(1046, 494) model=(709.469, -230.291) frame=80
    -> frames=39 duration=1.3 vector=(79.3327, -77.5019) s=110.906 v0=176.044 velocity=(125.926, -123.02)

    WHITE from collision to in rest:
    WHITE at collision: pixel=(973, 426) model=(583.705, -113.247) frame=41
    WHITE in rest: pixel=(951, 504) model=(545.906, -246.056) frame=89
    -> frames=48 duration=1.6 vector=(-37.7993, -132.809) s=138.084 v0=197.971 velocity=(-54.1931, -190.409)
    */
    const glm::vec2& whiteStart = glm::vec2{504.88, 77.68};
    const glm::vec2& redStart = glm::vec2 {630.137, -152.789};
    const glm::vec2& whiteCollision = glm::vec2{585.423, -114.962};
    const glm::vec2& redCollision = glm::vec2 {630.137, -152.789};
    const glm::vec2& redInRest = glm::vec2 {709.469, -230.291};
    const glm::vec2& whiteInRest = glm::vec2 {545.906, -246.056};
    const glm::vec2& redDirectionAfterCollision = redInRest - redCollision;
    const glm::vec2& whiteDirectionAfterCollision = whiteInRest - whiteCollision;
    const float collisionTime = 0.633333f;
    const float redInRestTime = 1.3f + collisionTime; // since collision
    const float whiteInRestTime = 1.6f + collisionTime; // since beginning
//    glm::vec2 velocity {144.223, -344.953};
    glm::vec2 velocity = calculateInitialVelocity(whiteCollision - whiteStart, 19);

    std::string white = "WHITE-1";
    std::string red = "RED-1";
    billiard::search::State state({
          billiard::search::Ball { whiteStart, "WHITE", white},
          billiard::search::Ball { redStart, "RED", red},
    });

    billiard::search::Configuration config = loadConfig("./resources/configuration.json");

    std::optional<System> systemOpt = billiard::search::simulate(state, velocity, config);
    if (!systemOpt) {
        std::cout << "No result" << std::endl;
        return;
    }
    System system = systemOpt.value();

    std::cout << "Layers: " << std::to_string(system._layers.size()) << std::endl;
    EXPECT_EQ(system._layers.size(), 6);

    Layer& startLayer = system._layers[0];
    auto whiteMovingNode = getNodeOfBall(startLayer, white, NodeType::BALL_SHOT)->toBallShot();
    float whiteStartPositionError = checkPosition(whiteMovingNode->_ball, whiteStart);
    EXPECT_FLOAT_EQ(whiteStartPositionError, 0.0f);

    auto redStartNode = getNodeOfBall(startLayer, red, NodeType::BALL_IN_REST)->toInRest();
    float redStartPositionError = checkPosition(redStartNode->_ball, redStart);
    EXPECT_FLOAT_EQ(redStartPositionError, 0.0f);

    Layer& whiteRollingLayer = system._layers[1];
    auto whiteRollingNode = getNodeOfBall(whiteRollingLayer, white, NodeType::BALL_MOVING)->toBallMoving();
    EXPECT_TRUE(whiteRollingNode);
    EXPECT_EQ(whiteRollingNode->_after._isRolling, true);

    Layer& collisionLayer = system._layers[2];
    auto whiteCollisionNode = getNodeOfBall(collisionLayer, white, NodeType::BALL_COLLISION)->toBallCollision();
    EXPECT_TRUE(whiteCollisionNode);
    float whiteCollisionPositionError = checkPosition(whiteCollisionNode->_after, whiteCollision);

    auto redCollisionNode = getNodeOfBall(collisionLayer, red, NodeType::BALL_COLLISION)->toBallCollision();
    EXPECT_TRUE(redCollisionNode);
    float redCollisionPositionError = checkPosition(redCollisionNode->_after, redCollision);

    Layer& redRollingLayer = system._layers[3];
    auto redRollingNode = getNodeOfBall(redRollingLayer, red, NodeType::BALL_MOVING)->toBallMoving();
    EXPECT_TRUE(redRollingNode);
    EXPECT_EQ(redRollingNode->_after._isRolling, true);

    Layer& redInRestLayer = system._layers[4];
    auto redInRestNode = getNodeOfBall(redInRestLayer, red, NodeType::BALL_IN_REST)->toInRest();
    EXPECT_TRUE(redInRestNode);
    float redInRestPositionError = checkPosition(redInRestNode->_ball, redInRest);
    float redInRestTimeError = checkTime(redInRestLayer, redInRestTime);
    float redInRestDirectionError = checkDirection(redInRestNode->_ball._position - redCollisionNode->_after._position, redDirectionAfterCollision);

    Layer& whiteInRestLayer = system._layers[5];
    auto whiteInRestNode = getNodeOfBall(whiteInRestLayer, white, NodeType::BALL_IN_REST)->toInRest();
    EXPECT_TRUE(whiteInRestNode);
    float whiteInRestPositionError = checkPosition(whiteInRestNode->_ball, whiteInRest);
    float whiteInRestTimeError = checkTime(whiteInRestLayer, whiteInRestTime);
    float whiteInRestDirectionError = checkDirection(whiteInRestNode->_ball._position - whiteCollisionNode->_after._position, whiteDirectionAfterCollision);

    std::cout << "Test " << name << " "
//              << "start pos ["
//              << "white=" << std::to_string(whiteStartPositionError) << " "
//              << "red=" << std::to_string(redStartPositionError)
//              << "]" << " "
//              << std::endl
              << "collision pos ["
              << "white=" << std::to_string(whiteCollisionPositionError) << " "
              << "red=" << std::to_string(redCollisionPositionError)
              << "]" << " "
              << std::endl
              << "red in rest ["
              << "pos=" << std::to_string(redInRestPositionError) << " "
              << "time=" << std::to_string(redInRestTimeError) << " "
              << "dir=" << std::to_string(redInRestDirectionError) << " "
              << "]" << " "
              << std::endl
              << "white in rest ["
              << "pos=" << std::to_string(whiteInRestPositionError) << " "
              << "time=" << std::to_string(whiteInRestTimeError) << " "
              << "dir=" << std::to_string(whiteInRestDirectionError) << " "
              << "]" << " "
              << std::endl;
}

TEST(SimulationVsReality, video_13_0030_0034) {

    std::string name = "video_13_0030_0034";
    /*
    WHITE, WHITE, 631.828, -149.426
    RED, RED, 771.566, -295.777
    V, 454.577, -468.858

    WHITE from start to collision:
    WHITE at start: pixel=(1001, 447) model=(631.828, -149.426) frame=27
    WHITE at collision: pixel=(1061, 509) model=(735.251, -256.098) frame=34
    -> frames=7 duration=0.233333 vector=(103.423, -106.672) s=148.577 v0=653.045 velocity=(454.577, -468.858)

    RED from collision to pocket:
    RED at collision: pixel=(1082, 532) model=(771.566, -295.777) frame=33
    RED in pocket: pixel=(1156, 620) model=(900.043, -448.081) frame=50
    -> frames=17 duration=0.566667 vector=(128.477, -152.304) s=199.256 v0=391.177 velocity=(252.226, -299.002)

    WHITE from collision to in rest:
    WHITE at collision: pixel=(1062, 511) model=(736.981, -259.534) frame=34
    WHITE in rest: pixel=(1094, 548) model=(792.353, -323.378) frame=63
    -> frames=29 duration=0.966667 vector=(55.3715, -63.8433) s=84.5102 v0=154.891 velocity=(101.485, -117.013)
    */
    const glm::vec2& whiteStart = glm::vec2{631.828, -149.426};
    const glm::vec2& redStart = glm::vec2 {771.566, -295.777};
    const glm::vec2& whiteCollision = glm::vec2{735.251, -256.098};
    const glm::vec2& redCollision = glm::vec2 {771.566, -295.777};
    const glm::vec2& whiteInRest = glm::vec2 {792.353, -323.378};
    const glm::vec2& whiteDirectionAfterCollision = whiteInRest - whiteCollision;
    const float collisionTime = 0.233333f;
    const float redPottedTime = 0.566667f + collisionTime; // since collision
    const float whiteInRestTime = 0.966667f + collisionTime; // since beginning
//    glm::vec2 velocity {454.577, -468.858};
    glm::vec2 velocity = calculateInitialVelocity(whiteCollision - whiteStart, 7);

    std::string white = "WHITE-1";
    std::string red = "RED-1";
    billiard::search::State state({
                                          billiard::search::Ball { whiteStart, "WHITE", white},
                                          billiard::search::Ball { redStart, "RED", red},
                                  });

    billiard::search::Configuration config = loadConfig("./resources/configuration.json");

    std::optional<System> systemOpt = billiard::search::simulate(state, velocity, config);
    if (!systemOpt) {
        std::cout << "No result" << std::endl;
        return;
    }
    System system = systemOpt.value();

    std::cout << "Layers: " << std::to_string(system._layers.size()) << std::endl;
    EXPECT_EQ(system._layers.size(), 6);

    Layer& startLayer = system._layers[0];
    auto whiteMovingNode = getNodeOfBall(startLayer, white, NodeType::BALL_SHOT)->toBallShot();
    float whiteStartPositionError = checkPosition(whiteMovingNode->_ball, whiteStart);
    EXPECT_FLOAT_EQ(whiteStartPositionError, 0.0f);

    auto redInRestNode = getNodeOfBall(startLayer, red, NodeType::BALL_IN_REST)->toInRest();
    float redStartPositionError = checkPosition(redInRestNode->_ball, redStart);
    EXPECT_FLOAT_EQ(redStartPositionError, 0.0f);

    Layer& whiteRollingLayer = system._layers[1];
    auto whiteRollingNode = getNodeOfBall(whiteRollingLayer, white, NodeType::BALL_MOVING)->toBallMoving();
    EXPECT_TRUE(whiteRollingNode);
    EXPECT_EQ(whiteRollingNode->_after._isRolling, true);

    Layer& collisionLayer = system._layers[2];
    auto whiteCollisionNode = getNodeOfBall(collisionLayer, white, NodeType::BALL_COLLISION)->toBallCollision();
    EXPECT_TRUE(whiteCollisionNode);
    float whiteCollisionPositionError = checkPosition(whiteCollisionNode->_after, whiteCollision);

    auto redCollisionNode = getNodeOfBall(collisionLayer, red, NodeType::BALL_COLLISION)->toBallCollision();
    EXPECT_TRUE(redCollisionNode);
    float redCollisionPositionError = checkPosition(redCollisionNode->_after, redCollision);

    Layer& redRollingLayer = system._layers[3];
    auto redRollingNode = getNodeOfBall(redRollingLayer, red, NodeType::BALL_MOVING)->toBallMoving();
    EXPECT_TRUE(redRollingNode);
    EXPECT_EQ(redRollingNode->_after._isRolling, true);

    Layer& redPottedLayer = system._layers[4];
    auto redPottedNode = getNodeOfBall(redPottedLayer, red, NodeType::BALL_POTTING)->toPotted();
    EXPECT_TRUE(redPottedNode);
    float redPottedTimeError = checkTime(redPottedLayer, redPottedTime);

    Layer& whiteInRestLayer = system._layers[5];
    auto whiteInRestNode = getNodeOfBall(whiteInRestLayer, white, NodeType::BALL_IN_REST)->toInRest();
    EXPECT_TRUE(whiteInRestNode);
    float whiteInRestPositionError = checkPosition(whiteInRestNode->_ball, whiteInRest);
    float whiteInRestTimeError = checkTime(whiteInRestLayer, whiteInRestTime);
    float whiteInRestDirectionError = checkDirection(whiteInRestNode->_ball._position - whiteCollisionNode->_after._position, whiteDirectionAfterCollision);

    std::cout << "Test " << name << " "
              //        << "start pos ["
              //        << "white=" << std::to_string(whiteStartPositionError) << " "
              //        << "red=" << std::to_string(redStartPositionError)
              //        << "]" << " "
              //        << std::endl
              << "collision pos ["
              << "white=" << std::to_string(whiteCollisionPositionError) << " "
              << "red=" << std::to_string(redCollisionPositionError)
              << "]" << " "
              << std::endl
              << "red potted ["
              << "time=" << std::to_string(redPottedTimeError) << " "
              << "]" << " "
              << std::endl
              << "white in rest ["
              << "pos=" << std::to_string(whiteInRestPositionError) << " "
              << "time=" << std::to_string(whiteInRestTimeError) << " "
              << "dir=" << std::to_string(whiteInRestDirectionError) << " "
              << "]" << " "
              << std::endl;
}
