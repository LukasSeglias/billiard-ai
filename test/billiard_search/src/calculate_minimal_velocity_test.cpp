#include <gtest/gtest.h>
#include <billiard_search/billiard_search.hpp>
#include <billiard_physics/billiard_physics.hpp>
#include "util.hpp"

using billiard::search::SearchNode;
using billiard::search::SearchActionType;
using billiard::search::PhysicalEvent;
using billiard::search::PhysicalEventType;

std::shared_ptr<billiard::search::SearchState> getState() {

    billiard::search::Configuration config;
    config._table.minimalPocketVelocity = 10;
    config._ball._radius = 26.15;
    config._ball._diameter = 52.3;

    auto state = std::make_shared<billiard::search::SearchState>();
    state->_config = config;

    return state;
}

glm::vec2 roll(const glm::vec2& startPosition,
               const glm::vec2& targetPosition,
               const glm::vec2& targetVelocity) {
    // Expected value v1:
    // v1 = sqrt(|v2|^2 + 2 * g * cR * s) * (v2/|v2|)
    glm::vec2 toTarget = targetPosition - startPosition;
    glm::vec2 velocityDirection = glm::normalize(targetVelocity);
    float v2Amount = glm::length(targetVelocity);
    float g = billiard::physics::gravitationalAcceleration;
    float cR = billiard::physics::frictionCoefficient;
    float s = glm::length(toTarget);
    float velocity = sqrt(v2Amount*v2Amount + 2 * g * cR * s);
    return velocity * velocityDirection;
}

glm::vec2 rollIntoPocket(const glm::vec2& pocketPosition, const glm::vec2& ballPosition, std::shared_ptr<billiard::search::SearchState> state) {
    // Expected value v1:
    // v1 = sqrt(|v2|^2 + 2 * g * cR * s) * (v2/|v2|)
    glm::vec2 toTarget = pocketPosition - ballPosition;
    glm::vec2 velocityDirection = glm::normalize(toTarget);
    float v2Amount = state->_config._table.minimalPocketVelocity;
    float g = billiard::physics::gravitationalAcceleration;
    float cR = billiard::physics::frictionCoefficient;
    float s = glm::length(toTarget);
    float velocity = sqrt(v2Amount*v2Amount + 2 * g * cR * s);
    return velocity * velocityDirection;
}

TEST(calculateMinimalVelocity, white_goes_directly_into_pocket) {

    auto state = getState();

    glm::vec2 pocketPosition {900, 450};
    glm::vec2 whitePosition {0, 0};

    glm::vec2 expected = rollIntoPocket(pocketPosition, whitePosition, state);

    std::shared_ptr<SearchNode> pocket = createNode(SearchActionType::NONE, "", {}, nullptr);
    std::shared_ptr<SearchNode> white = createNode(SearchActionType::DIRECT, "WHITE-1", {
            PhysicalEvent { PhysicalEventType::POCKET_COLLISION, pocketPosition },
            PhysicalEvent { PhysicalEventType::BALL_KICK, whitePosition }
    }, pocket);

    glm::vec2 minimalVelocity = billiard::search::calculateMinimalVelocity(white->currentPath(), state);

    std::cout << "Calculated minimal velocity: (" << minimalVelocity.x << ", " << minimalVelocity.y << ") length: " << glm::length(minimalVelocity) << std::endl;
    std::cout << "Expected minimal velocity: (" << expected.x << ", " << expected.y << ") length: " << glm::length(expected) << std::endl;
}

TEST(calculateMinimalVelocity, white_pushes_red_directly_into_pocket) {

    auto state = getState();

    glm::vec2 pocketPosition {900, 450};
    glm::vec2 whitePosition {0, 0};
    glm::vec2 redPosition = 0.5f * (pocketPosition - whitePosition);
    glm::vec2 whiteRedCollisionPosition = redPosition - state->_config._ball._diameter * glm::normalize(redPosition - whitePosition);

    glm::vec2 redToPocket = rollIntoPocket(pocketPosition, redPosition, state);
    glm::vec2 whiteRedCollision = redToPocket; // Elastic collision does not remove energy
    glm::vec2 whiteToRed = roll(whitePosition, whiteRedCollisionPosition, whiteRedCollision);
    glm::vec2 expected = whiteToRed;

    std::shared_ptr<SearchNode> pocket = createNode(SearchActionType::NONE, "", {}, nullptr);
    std::shared_ptr<SearchNode> red = createNode(SearchActionType::DIRECT, "RED-1", {
            PhysicalEvent { PhysicalEventType::POCKET_COLLISION, pocketPosition },
            PhysicalEvent { PhysicalEventType::BALL_KICK, redPosition }
    }, pocket);
    std::shared_ptr<SearchNode> white = createNode(SearchActionType::DIRECT, "WHITE-1", {
            PhysicalEvent { PhysicalEventType::BALL_COLLISION, whiteRedCollisionPosition },
            PhysicalEvent { PhysicalEventType::BALL_KICK, whitePosition }
    }, red);

    glm::vec2 minimalVelocity = billiard::search::calculateMinimalVelocity(white->currentPath(), state);

    std::cout << "Calculated minimal velocity: (" << minimalVelocity.x << ", " << minimalVelocity.y << ") length: " << glm::length(minimalVelocity) << std::endl;
    std::cout << "Expected minimal velocity: (" << expected.x << ", " << expected.y << ") length: " << glm::length(expected) << std::endl;
}
