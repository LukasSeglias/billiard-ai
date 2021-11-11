#include <billiard_search/billiard_search.hpp>
#include <billiard_physics/billiard_physics.hpp>
#include <billiard_debug/billiard_debug.hpp>
#include <process/process.hpp>
#include <memory>
#include <optional>
#include <iostream>
#include <glm/gtx/string_cast.hpp>

namespace billiard::search {

    #define PROCESSES 1
    #define SYNC_PERIOD_MS 100
    #define BREAKS 1
    #define BANK_INDIRECTION 0
    // TODO: find a good number
    #define FORWARD_SEARCHES 3
    // TODO: find a good number
    #define MAX_VELOCITY_TO_ADD 50.0f
    // TODO: find a good number
    #define MAX_VELOCITY_SQUARED 10000.0f * 10000.0f
    #define MAX_EVENTS 30
    #define VELOCITY_STEP (MAX_VELOCITY_TO_ADD / FORWARD_SEARCHES)
    #define COST_FLOAT_TO_INT_FACTOR 100000
    // 1.0 => simulation and search cost are both equally weighted. 0.5 => simulation cost is weighted half the search cost.
    #define SIMULATION_COST_FRACTION_OF_SEARCH_COST 1.0
    // Maximum search depth allowed before cutoff
    #define MAX_SEARCH_DEPTH 3
    // Maximum number of ball collisions to be considered in search of a path
    #define SEARCH_MAX_BALL_COLLISIONS 2

    struct SearchNode;
    std::vector<node::System> mapToSolution(const std::shared_ptr<SearchNode>& solution);
    std::vector<std::shared_ptr<SearchNode>>
    expand(const std::shared_ptr<SearchNode>& input, const std::shared_ptr<SearchState>& config);
    std::vector<std::shared_ptr<SearchNode>>
    simulate(const std::shared_ptr<SearchNode>& input, const std::shared_ptr<SearchState>& config, bool mustBeValid = true);
    std::vector<std::shared_ptr<SearchNode>>
    expandSearchNode(const std::shared_ptr<SearchNode>& input, const std::shared_ptr<SearchState>& config);
    std::vector<std::shared_ptr<SearchNode>>
    addPocketsAsInitialTargets(const State& state, const Search& search, const Configuration& config);
    node::Layer toInputLayer(const std::shared_ptr<SearchNode>& input, const std::string& cueBallId, const glm::vec2 force, float accelerationLength,
                             float slideAccelerationLength);
}

std::future<std::vector<std::vector<billiard::search::node::System>>> EXPORT_BILLIARD_SEARCH_LIB
billiard::search::search(const billiard::search::State& state, const billiard::search::Search& search,
                         uint16_t solutions, const Configuration& config) {
    static process::ProcessManager<std::shared_ptr<SearchNode>, SearchState, std::vector<node::System>> processManager {
            PROCESSES,
            SYNC_PERIOD_MS,
            expand,
            mapToSolution};

    auto searchState = std::make_shared<SearchState>(SearchState{config});
    searchState->_ball._accelerationLength = billiard::physics::accelerationLength();
    searchState->_ball._slideAccelerationLength = billiard::physics::slideAccelerationLength();

    return processManager.process(addPocketsAsInitialTargets(state, search, config), solutions,searchState);
}

std::future<std::vector<std::shared_ptr<billiard::search::SearchNode>>> EXPORT_BILLIARD_SEARCH_LIB
billiard::search::searchOnly(const billiard::search::State& state, const billiard::search::Search& search,
                             uint16_t solutions, const Configuration& config) {
    static process::ProcessManager<std::shared_ptr<SearchNode>, SearchState, std::shared_ptr<SearchNode>> processManager {
            1,
            SYNC_PERIOD_MS,
            expand,
            [](std::shared_ptr<SearchNode> searchNode) {
                return searchNode;
    }};

    auto searchState = std::make_shared<SearchState>(SearchState{config});
    searchState->_ball._accelerationLength = billiard::physics::accelerationLength();
    searchState->_ball._slideAccelerationLength = billiard::physics::slideAccelerationLength();

    return processManager.process(addPocketsAsInitialTargets(state, search, config), solutions,
                                  std::make_shared<SearchState>(SearchState{config}));
}

std::optional<billiard::search::node::System>
billiard::search::simulate(const State& state, const glm::vec2& velocity, const Configuration& config) {

    std::unordered_map<std::string, Ball> ballState;
    std::string cueBallId;
    for (auto& ball : state._balls) {
        ballState.insert({ball._id, ball});

        if (ball._type == "WHITE") {
            cueBallId = ball._id;
        }
    }

    auto stateNode = SearchNode::search();
    auto stateNodeSearch = stateNode->asSearch();
    stateNodeSearch->_state = ballState;

    auto layer = toInputLayer(stateNode, cueBallId, velocity, billiard::physics::accelerationLength(),
                              billiard::physics::slideAccelerationLength());
    auto simulationNode = SearchNode::simulation(stateNode);
    auto simulationSearchNode = simulationNode->asSimulation();
    simulationSearchNode->_simulation.append(layer);

    auto searchState = std::make_shared<SearchState>(SearchState{config});
    searchState->_ball._accelerationLength = billiard::physics::accelerationLength();
    searchState->_ball._slideAccelerationLength = billiard::physics::slideAccelerationLength();

    auto simulations = simulate(simulationNode, searchState, false);

    if (!simulations.empty()) {
        return simulations.at(0)->asSimulation()->_simulation;
    }

    return std::nullopt;
}

///////////////////////////////////////////////////////////////////////
//// PRIVATE IMPLEMENTATION
///////////////////////////////////////////////////////////////////////

namespace billiard::search {

#ifdef BILLIARD_DEBUG
    std::string readable(const PhysicalEventType& eventType);
    std::string readable(const event::EventType& eventType);
    std::string readable(const node::NodeType& nodeType);
    std::string readable(const SearchActionType& actionType);
    void logBalls(const std::unordered_map<std::string, node::Node>& balls);
    std::string logPath(const std::vector<const SearchNode *>& path);
    std::ostream& operator<<(std::ostream& os, const glm::vec4& vector);
    std::ostream& operator<<(std::ostream& os, const glm::vec2& vector);
    std::ostream& operator<<(std::ostream& os, const PhysicalEvent& event);
    std::ostream& operator<<(std::ostream& os, const std::vector<PhysicalEvent>& events);
    std::ostream& operator<<(std::ostream& os, const state::BallState& ball);
    std::ostream& operator<<(std::ostream& os, const node::Node& node);
    std::ostream& operator<<(std::ostream& os, const std::set<std::string>& set);
    std::ostream& operator<<(std::ostream& os, const std::optional<event::Event>& event);
    std::ostream& operator<<(std::ostream& os, const node::Layer& layer);
#endif

    std::vector<node::System> mapToSolution(const std::shared_ptr<SearchNode>& solution) {
        return solution->allSimulations();
    }

    std::vector<std::shared_ptr<SearchNode>>
    expand(const std::shared_ptr<SearchNode>& input, const std::shared_ptr<SearchState>& state) {
        if (input->_type == SearchNodeType::SEARCH) {

            if (input->_searchDepth < MAX_SEARCH_DEPTH) {
                return expandSearchNode(input, state);
            } else {
                DEBUG("[expand] Maximum search depth reached for node "
                      << input->asSearch()->_ballId
                      << " path: " << logPath(input->currentPath())
                      << " type=" << readable(input->asSearch()->_action)
                      << " cost=" << input->_cost
                      << " searchDepth=" << input->_searchDepth
                      << std::endl);
                return {};
            }

        } else {
            return simulate(input, state);
        }
    }

    std::vector<std::shared_ptr<SearchNode>>
    addPocketsAsInitialTargets(const State& state, const Search& search, const Configuration& config) {
        std::vector<std::shared_ptr<SearchNode>> expanded;

        std::unordered_map<std::string, Ball> ballState;
        for (auto& ball : state._balls) {
            ballState.insert({ball._id, ball});
        }

        for (auto& pocket : config._table._pockets) {
            auto node = SearchNode::search();
            auto nodeSearch = node->asSearch();

            nodeSearch->_state = ballState;
            nodeSearch->_search = search;
            nodeSearch->_action = SearchActionType::NONE;
            nodeSearch->_ballId = pocket._id;
            nodeSearch->_unusedBalls = ballState;

            DEBUG("[addPocketsAsInitialTargets] Place pocket " << pocket._id << " at " << pocket._position << std::endl);

            expanded.emplace_back(node);
        }

        return expanded;
    }

    ///////////////////////////////////////////////////////////////////////
    //// EXPAND SearchNode
    ///////////////////////////////////////////////////////////////////////

    std::vector<std::shared_ptr<SearchNode>>
    expandSearchNodeByBalls(const std::shared_ptr<SearchNode>& input, const std::shared_ptr<SearchState>& state);
    std::vector<std::shared_ptr<SearchNode>>
    expandSearchNodeByBall(const std::shared_ptr<SearchNode>& input, const std::shared_ptr<SearchState>& state,
                         const std::shared_ptr<SearchNodeSearch>& searchInput, const std::pair<std::string, Ball>& ball);
    std::vector<std::shared_ptr<SearchNode>>
    prepareForSimulation(const std::shared_ptr<SearchNode>& input, const std::string& cueBallId, const std::shared_ptr<SearchState>& state);
    std::vector<std::shared_ptr<SearchNode>>
    expandSearchNodeByBanks(const std::shared_ptr<SearchNode>& input, const std::shared_ptr<SearchState>& state);
    std::vector<std::shared_ptr<SearchNode>>
    expandSearchNodeByBank(const std::shared_ptr<SearchNode>& input, const std::shared_ptr<SearchState>& state,
                         const std::shared_ptr<SearchNodeSearch>& searchInput, uint8_t depth);
    uint64_t searchCost(const std::vector<PhysicalEvent>& events,
                        const Ball& ball,
                        const std::shared_ptr<SearchNodeSearch>& parentSearchNode,
                        const std::shared_ptr<SearchState>& state);
    std::vector<std::shared_ptr<SearchNode>>
    expandSearchNodeByBankBall(const std::shared_ptr<SearchNode>& input, const std::shared_ptr<SearchState>& state,
                             const std::shared_ptr<SearchNodeSearch>& searchInput, uint8_t depth,
                             const std::pair<std::string, Ball>& ball);

    std::vector<std::shared_ptr<SearchNode>>
    expandSearchNode(const std::shared_ptr<SearchNode>& input, const std::shared_ptr<SearchState>& state) {
        std::vector<std::shared_ptr<SearchNode>> expanded;

        auto expandedByBall = expandSearchNodeByBalls(input, state);
        auto expandedByBank = expandSearchNodeByBanks(input, state);

        expanded.insert(expanded.end(), expandedByBall.begin(), expandedByBall.end());
        expanded.insert(expanded.end(), expandedByBank.begin(), expandedByBank.end());

        return expanded;
    }

    std::vector<std::shared_ptr<SearchNode>>
    expandSearchNodeByBalls(const std::shared_ptr<SearchNode>& input, const std::shared_ptr<SearchState>& state) {
        std::vector<std::shared_ptr<SearchNode>> expanded;
        auto searchInput = input->asSearch();
        auto& unused = searchInput->_unusedBalls;
        for (auto& ball : unused) {

            if ((/* Parent is pocket and ball is either the ball or one of the balls we want to pocket */
                    searchInput->_action == SearchActionType::NONE
                    && (ball.first == searchInput->_search._id || std::count(searchInput->_search._types.begin(), searchInput->_search._types.end(), ball.second._type))
                ) || searchInput->_action != SearchActionType::NONE) {

                DEBUG("[expandSearchNodeByBalls] Expand " << ball.first
                        << " child of " << input->asSearch()->_ballId
                        << " path: " << logPath(input->currentPath())
                        << " type=" << readable(input->asSearch()->_action)
                        << " cost=" << input->_cost
                        << " searchDepth=" << input->_searchDepth
                        << std::endl);

                auto result = expandSearchNodeByBall(input, state, searchInput, ball);
                expanded.insert(expanded.end(), result.begin(), result.end());
            }

//            if ((searchInput->_action != SearchActionType::NONE && ball.second._type == "WHITE") ||
////                (searchInput->_action == SearchActionType::NONE && (ball.first == searchInput->_search._id || ball.second._type == searchInput->_search._type))) {
//                ((ball.first == searchInput->_search._id || ball.second._type == searchInput->_search._type))) {
//
//                DEBUG("[expandSearchNodeByBalls] Expand " << ball.first << " child of " << input->asSearch()->_ballId << " type " << input->asSearch()->_action  << std::endl);
//
//                auto result = expandSearchNodeByBall(input, state, searchInput, ball);
//                expanded.insert(expanded.end(), result.begin(), result.end());
//            }
        }

        return expanded;
    }

    std::optional<Pocket> getPocketById(const std::shared_ptr<SearchState>& state, const std::string& id) {

        for (auto& pocket : state->_config._table._pockets) {
            if (pocket._id == id) {
                return std::make_optional(pocket);
            }
        }
        return std::nullopt;
    }

    std::optional<Rail> getRailById(const std::shared_ptr<SearchState>& state, const std::string& id) {

        for (auto& rail : state->_config._table._rails) {
            if (rail._id == id) {
                return std::make_optional(rail);
            }
        }
        return std::nullopt;
    }

    std::optional<PhysicalEvent> getLastEvent(const std::shared_ptr<SearchNodeSearch>& node) {
        auto& events = node->_events;
        if (events.empty()) {
            return std::nullopt;
        } else {
            int lastIndex = events.size() - 1;
            return std::make_optional(events[lastIndex]);
        }
    }

    std::optional<PhysicalEvent> getSecondLastEvent(const std::shared_ptr<SearchNodeSearch>& node) {
        auto& events = node->_events;
        if (events.size() < 2) {
            return std::nullopt;
        } else {
            int secondLastIndex = events.size() - 2;
            return std::make_optional(events[secondLastIndex]);
        }
    }

    /*
     * TEST
     * WHITE0, WHITE, 0, 0
     * RED2, RED, 62.31, -150
     * RED1, RED, 0, -300
     */
    bool collidesOnTheWay(const std::shared_ptr<SearchNodeSearch>& parent,
                          const std::shared_ptr<SearchState>& state,
                          const Ball& ball,
                          const std::string& targetBallId,
                          const glm::vec2& targetPosition) {

        auto& ballId = ball._id;

        for (auto& other : parent->_state) {

            if (other.first == ballId || other.first == targetBallId) {
                continue;
            }

            // a = ballDiameter
            // b = spacing
            // minDistance = a + b
            // minSquaredDistance = (a + b)^2 = a^2 + 2ab + b^2
            // 2ab = 2 * ballDiameter * spacing, b^2 = spacing^2
            static float spacing = 10.0f;
            static float minSquaredDistance = state->_config._ball._diameterSquared +
                                            2 * state->_config._ball._diameter * spacing +
                                            spacing * spacing;
            float squaredDistance = billiard::physics::pointToLineSegmentSquaredDistance(ball._position, targetPosition,
                                                                                         other.second._position);
            if (squaredDistance <= minSquaredDistance) {

                DEBUG("Ball " << ball._id
                << " collides on the way to " << targetPosition
                << " with ball " << other.first
                << " squaredDistance=" << squaredDistance
                << std::endl);
                return true;
            }
        }
        return false;
    }

    std::shared_ptr<SearchNode> pocketCollision(const std::shared_ptr<SearchNode>& parentNode,
                                                const std::shared_ptr<SearchState>& state,
                                                const std::shared_ptr<SearchNodeSearch>& parent,
                                                const Ball& ball) {

        std::string& pocketId = parent->_ballId;
        auto pocket = getPocketById(state, pocketId);
        assert(pocket);

        // TODO: check if that is possible

        // Only possible to hit ball if no other ball is on the way
        if (collidesOnTheWay(parent, state, ball, "", pocket->_position)) {
            return nullptr;
        }

        // TODO: check if collides with rail on the way

        auto result = SearchNode::search(parentNode);
        auto resultSearchNode = result->asSearch();
        resultSearchNode->_events.push_back(PhysicalEvent { PhysicalEventType::POCKET_COLLISION, pocket->_position, pocket->_id });
        resultSearchNode->_events.push_back(PhysicalEvent { PhysicalEventType::BALL_KICK, ball._position, "" });

        return result;
    }

    std::shared_ptr<SearchNode> ballCollision(const std::shared_ptr<SearchNode>& parentNode,
            const std::shared_ptr<SearchState>& state,
            const std::shared_ptr<SearchNodeSearch>& parent,
            const Ball& ball) {
        static glm::vec2 zero{0, 0};

        auto& ballPosition = ball._position;

        auto& targetBall = parent->_state.at(parent->_ballId);

        auto lastEvent = getLastEvent(parent);
        auto secondLastEvent = getSecondLastEvent(parent);
        assert(lastEvent);
        assert(secondLastEvent);
        glm::vec2 targetBallTargetVector = secondLastEvent->_targetPosition - lastEvent->_targetPosition;
        assert(targetBallTargetVector != zero);
        glm::vec2 targetBallTargetDirection = glm::normalize(targetBallTargetVector);
        float ballRadius = state->_config._ball._radius;
        glm::vec2 targetPosition = billiard::physics::elasticCollisionTargetPosition(lastEvent->_targetPosition, targetBallTargetDirection, ballRadius);

        // Only possible to hit ball from "behind"
        auto targetToBall = ballPosition - targetPosition;
        assert(targetToBall != zero);
        float sign = glm::dot(targetBallTargetDirection, glm::normalize(targetToBall));
        if (sign >= 0) { // TODO: find better
            return nullptr;
        }

        // Only possible to hit ball if no other ball is on the way
        if (collidesOnTheWay(parent, state, ball, targetBall._id, targetPosition)) {
            return nullptr;
        }

        // TODO: check if that is possible further

        auto result = SearchNode::search(parentNode);
        auto resultSearchNode = result->asSearch();
        resultSearchNode->_events.push_back(PhysicalEvent { PhysicalEventType::BALL_COLLISION, targetPosition, targetBall._id });
        resultSearchNode->_events.push_back(PhysicalEvent { PhysicalEventType::BALL_KICK, ball._position, "" });

        return result;
    }

    /*
     * TEST: State 1 - Alle Kugeln untereinander
     * RED2, RED, 0, 114.6
     * WHITE0, WHITE, 0, 0
     * RED1, RED, 0, -52.31
     *
     * TEST: State 2 - Kugel neben Queue Bahn
     * RED2, RED, 36.15, 52.3
     * WHITE0, WHITE, 0, 0
     * RED1, RED, 0, -52.31
     *
     * TEST: Kombi
     * RED2, RED, 36.15, 52.3
     * RED3, RED, -36.15, 52.3
     * RED4, RED, 0, 114.6
     * WHITE0, WHITE, 0, 0
     * RED1, RED, 0, -52.31
     *
     */
    bool enoughSpace(const std::shared_ptr<SearchNode>& input,
                     const std::shared_ptr<SearchState>& state,
                     const std::shared_ptr<SearchNodeSearch>& parent,
                     const std::pair<std::string, Ball>& ball) {
        std::string agent = "[enoughSpace] ";
        auto lengthS = state->_config._ball._diameter + state->_config._ball._radius;
        auto lengthE = 10; // [mm]
        auto minDistance = lengthE + state->_config._ball._radius;
        auto minDistanceSquared = minDistance * minDistance;

        assert(parent->_events.size() >= 2);

        auto lastEvent = parent->_events.at(parent->_events.size() - 1);
        auto secondLastEvent = parent->_events.at(parent->_events.size() - 2);
        auto s = -lengthS * glm::normalize(secondLastEvent._targetPosition - lastEvent._targetPosition);
        auto targetPoint = parent->_state.at(ball.first)._position + s;

        return std::all_of(parent->_state.begin(), parent->_state.end(),
                           [&ball, &targetPoint, &minDistanceSquared, &agent](
                                   const std::pair<std::string, Ball>& otherBall) -> bool {
                               if (ball.first != otherBall.first) {
                                   auto squaredDistance = billiard::physics::pointToLineSegmentSquaredDistance(
                                           ball.second._position, targetPoint, otherBall.second._position);
                                   if (squaredDistance < minDistanceSquared) {
                                       DEBUG(agent << "squared distance " << squaredDistance << " "
                                                   << "of ball " << otherBall.first << " "
                                                   << "is closer than minimum squared distance " << minDistanceSquared
                                                   << std::endl);
                                       return false;
                                   }
                               }
                               return true;
                           });
    }

    std::shared_ptr<SearchNode> expandBallIfPossible(const std::shared_ptr<SearchNode>& input,
                                                     const std::shared_ptr<SearchState>& state,
                                                     const std::shared_ptr<SearchNodeSearch>& searchInput,
                                                     const std::pair<std::string, Ball>& ball) {

        auto& parent = searchInput;

        if (ball.second._type == "WHITE" && !enoughSpace(input, state, parent, ball)) {
            return nullptr;
        }

        if (parent->_action == SearchActionType::NONE) {
            // Must hit pocket
            return pocketCollision(input, state, parent, ball.second);
        } else {
            // Must hit other ball (= targetBall)
            return ballCollision(input, state, parent, ball.second);
        }
    }

    std::vector<std::shared_ptr<SearchNode>>
    expandSearchNodeByBall(const std::shared_ptr<SearchNode>& input, const std::shared_ptr<SearchState>& state,
                         const std::shared_ptr<SearchNodeSearch>& searchInput,
                         const std::pair<std::string, Ball>& ball) {
        std::vector<std::shared_ptr<SearchNode>> expanded;

        auto result = expandBallIfPossible(input, state, searchInput, ball);

        if (result) {
            auto resultSearchNode = result->asSearch();
            resultSearchNode->_ballId = ball.first;
            resultSearchNode->_unusedBalls.erase(resultSearchNode->_unusedBalls.find(ball.first));
            auto parentSearchNode = result->_parent? result->_parent->asSearch() : nullptr;
            uint64_t searchStepCost = searchCost(resultSearchNode->_events, ball.second, parentSearchNode, state);
            result->_cost = input->_cost + searchStepCost;
            result->_searchCost = input->_searchCost + searchStepCost;

            DEBUG("[expandSearchNodeByBall] " << "expanded" << " "
                                              << "ball=" << resultSearchNode->_ballId << " "
                                              << "cost=" << std::to_string(result->_cost) << " "
                                              << "searchCost=" << std::to_string(result->_searchCost) << " "
                                              << "action=" << resultSearchNode->_action << " "
                                              << "events=" << resultSearchNode->_events << " "
                                              << "parent=" << result->_parent->asSearch()->_ballId << " "
                                              << std::endl);

            if (ball.second._type == "WHITE") {
                auto prepared = prepareForSimulation(result, ball.first, state); // TODO: Comment if search test
                expanded.insert(expanded.end(), prepared.begin(), prepared.end());
                /*result->_isSolution = true;
                expanded.push_back(result);*/
            } else {
                expanded.push_back(result);
            }
        }

        return expanded;
    }

    glm::vec2
    calculateMinimalVelocity(const std::vector<const SearchNode*>& nodes, const std::shared_ptr<SearchState>& state) {
        // TODO: @performance: cancel early when minimal velocity is already over maximum if we gain perf from that?
        static glm::vec2 zero{0, 0};
        float minimalVelocityInPocket = state->_config._table.minimalPocketVelocity; // TODO: find a good number
        assert(minimalVelocityInPocket > 0.0f);
        glm::vec2 minimalVelocity { 0, 0 };

        std::string agent = "[calculateMinimalVelocity " + std::to_string(rand()) + "]";

        PhysicalEvent* previousEvent = nullptr;

        for (int nodeIndex = nodes.size() - 1; nodeIndex >= 0; nodeIndex--) {

            auto node = nodes[nodeIndex];
            auto searchNode = node->asSearch();

            if (searchNode->_action != SearchActionType::NONE) {

                for (auto& event : searchNode->_events) {

                    if (previousEvent) {

                        DEBUG(agent << " " << "previous: " << *previousEvent << " current: " << event << std::endl);

                        if (event._type == PhysicalEventType::BALL_COLLISION) {

                            // Collide with other ball
                            // Handled with next event, see below

                        } else if (previousEvent->_type == PhysicalEventType::BALL_COLLISION) {

                            // Collide with other ball
                            glm::vec2& currentPosition = event._targetPosition;
                            glm::vec2& targetPosition = previousEvent->_targetPosition;
                            glm::vec2 toTarget = targetPosition - currentPosition;
                            assert(toTarget != zero);
                            glm::vec2 originVelocity = glm::normalize(toTarget);
                            minimalVelocity = billiard::physics::elasticCollisionReverse(minimalVelocity, originVelocity);

                            // Roll to collision point
                            float distance = glm::length(toTarget);
                            minimalVelocity = billiard::physics::startVelocity(minimalVelocity, distance);

                            DEBUG(agent << " " << "Ball collision:"
                                                                << " distance to collision point: " << distance
                                                                << " from: " << currentPosition
                                                                << " to: " << targetPosition << std::endl);

                        } else if (previousEvent->_type == PhysicalEventType::POCKET_COLLISION) {

                            // Roll into pocket
                            glm::vec2& currentPosition = event._targetPosition;
                            glm::vec2& targetPosition = previousEvent->_targetPosition;
                            glm::vec2 toTarget = targetPosition - currentPosition;
                            assert(toTarget != zero);
                            float distance = glm::length(toTarget);
                            glm::vec2 finalVelocity = minimalVelocityInPocket * glm::normalize(toTarget);
                            minimalVelocity = billiard::physics::startVelocity(finalVelocity, distance);

                            DEBUG(agent << " " << "Roll into pocket:"
                                    << " distance: " << distance
                                    << " from: " << currentPosition
                                    << " to: " << targetPosition << std::endl);

                        } else {

                            // Roll some distance between two points
                            glm::vec2& currentPosition = event._targetPosition;
                            glm::vec2& targetPosition = previousEvent->_targetPosition;
                            glm::vec2 toTarget = targetPosition - currentPosition;
                            float distance = glm::length(toTarget);
                            minimalVelocity = billiard::physics::startVelocity(minimalVelocity, distance);

                            DEBUG(agent << " " << "Roll somewhere:"
                                << " distance: " << distance
                                << " from: " << currentPosition
                                << " to: " << targetPosition << std::endl);
                        }
                    }

                    previousEvent = &event;
                }
            }
        }

        DEBUG(agent << " " << "Result: " << minimalVelocity << std::endl);

        return minimalVelocity;
    }

    node::Layer toInputLayer(const std::shared_ptr<SearchNode>& input, const std::string& cueBallId, const glm::vec2 force,
                             float accelerationLength, float slideAccelerationLength) {
        std::unordered_map<std::string, node::Node> nodes;

        auto asSearch = input->asSearch();

        for(auto& ball : asSearch->_state) {
            if (ball.first == cueBallId) {
                node::BallShotNode initialEnergyNode{
                        state::BallState{
                                ball.second._position,
                                force,
                                accelerationLength,
                                slideAccelerationLength,
                                false
                                }
                };
                node::Node result{node::NodeType::BALL_SHOT, ball.second._type, node::NodeVariant(initialEnergyNode)};
                nodes.insert({ball.first, result});
            } else {
                node::BallInRestNode inRestNode{
                        state::BallState{
                                ball.second._position,
                                glm::vec2{0, 0},
                                accelerationLength,
                                slideAccelerationLength,
                                false
                        }
                };
                node::Node result{node::NodeType::BALL_IN_REST, ball.second._type, node::NodeVariant(inRestNode)};
                nodes.insert({ball.first, result});
            }
        }

        return node::Layer{0, nodes};
    }

    std::vector<std::shared_ptr<SearchNode>>
    prepareForSimulation(const std::shared_ptr<SearchNode>& input, const std::string& cueBallId, const std::shared_ptr<SearchState>& state) {
        std::vector<std::shared_ptr<SearchNode>> expanded;

        static glm::vec2 zero{0, 0};
        auto path = input->currentPath();
        auto minimalVelocity = calculateMinimalVelocity(path, state);
        assert(minimalVelocity != zero);
        auto minimalVelocityNormalized = glm::normalize(minimalVelocity);

        DEBUG("[prepareForSimulation] Found path: " << logPath(path) << std::endl);

        for (int i = 0; i < FORWARD_SEARCHES; ++i) {
            glm::vec2 increasedVelocity = minimalVelocity + (i * VELOCITY_STEP * minimalVelocityNormalized);
            if (glm::dot(increasedVelocity, increasedVelocity) > MAX_VELOCITY_SQUARED) {
                DEBUG("[prepareForSimulation] Start velocity " << increasedVelocity << " is too high" << std::endl);
                break;
            }

            auto layer = toInputLayer(input, cueBallId, increasedVelocity, state->_ball._accelerationLength,
                                      state->_ball._slideAccelerationLength);

            auto output = SearchNode::simulation(input);
            auto simulationSearchNode = output->asSimulation();
            simulationSearchNode->_simulation.append(layer);

            expanded.emplace_back(output);
        }
        return expanded;
    }

    std::vector<std::shared_ptr<SearchNode>> expandSearchNodeByBanks(const std::shared_ptr<SearchNode>& input,
                                                                   const std::shared_ptr<SearchState>& state) {
        std::vector<std::shared_ptr<SearchNode>> expanded;
        auto searchInput = input->asSearch();

        for(int depth = 1; depth < BANK_INDIRECTION; depth++) {
            auto expandedByBank = expandSearchNodeByBank(input, state, searchInput, depth);
            expanded.insert(expanded.end(), expandedByBank.begin(), expandedByBank.end());
        }

        return expanded;
    }

    std::vector<std::shared_ptr<SearchNode>>
    expandSearchNodeByBank(const std::shared_ptr<SearchNode>& input, const std::shared_ptr<SearchState>& state,
                         const std::shared_ptr<SearchNodeSearch>& searchInput, uint8_t depth) {
        std::vector<std::shared_ptr<SearchNode>> expanded;

        for (auto& ball : searchInput->_unusedBalls) {
            if (searchInput->_action != SearchActionType::NONE || (ball.first == searchInput->_search._id ||
                std::count(searchInput->_search._types.begin(), searchInput->_search._types.end(), ball.second._type))) {
                auto result = expandSearchNodeByBankBall(input, state, searchInput, depth, ball);
                expanded.insert(expanded.end(), result.begin(), result.end());
            }
        }

        return expanded;
    }

    std::vector<std::shared_ptr<SearchNode>>
    expandSearchNodeByBankBall(const std::shared_ptr<SearchNode>& input, const std::shared_ptr<SearchState>& state,
                               const std::shared_ptr<SearchNodeSearch>& searchInput, uint8_t depth,
                               const std::pair<std::string, Ball>& ball) {
        std::vector<std::shared_ptr<SearchNode>> expanded;

        auto result = SearchNode::search(input); // TODO: Remove and try to get result by expansion of ball over bank by depth. Use the last entry in "_backwardSearch".

        if (result) {
            auto resultSearchNode = result->asSearch();
            resultSearchNode->_unusedBalls.erase(resultSearchNode->_unusedBalls.find(ball.first));
            auto parentSearchNode = result->_parent? result->_parent->asSearch() : nullptr;
            uint64_t searchStepCost = searchCost(resultSearchNode->_events, ball.second, parentSearchNode, state);
            result->_cost = input->_cost + searchStepCost;
            result->_searchCost = input->_searchCost + searchStepCost;

            if (ball.second._type == "WHITE") {
                auto prepared = prepareForSimulation(result, ball.first, state);
                expanded.insert(expanded.end(), prepared.begin(), prepared.end());
            } else {
                expanded.push_back(result);
            }
        }

        return expanded;
    }

    double weightedDistance(double distanceSquared,
                            double optimalDistanceSquared,
                            double optimalDistanceCost,
                            double maxDistanceSquared) {

        if (distanceSquared < optimalDistanceSquared) {
            // Line starting at x=0, y=1 going to x=optimalDistanceSquared, y=optimalDistanceCost
            return (optimalDistanceCost-1)/optimalDistanceSquared * distanceSquared + 1;
        } else {
            // Line starting at x=optimalDistanceSquared, y=optimalDistanceCost going to x=maxDistanceSquared, y=1
            return (1-optimalDistanceCost)/(maxDistanceSquared-optimalDistanceSquared) * distanceSquared;
        }
    }

    uint64_t searchCost(const std::vector<PhysicalEvent>& events,
                        const Ball& ball,
                        const std::shared_ptr<SearchNodeSearch>& parentSearchNode,
                        const std::shared_ptr<SearchState>& state) {

        std::string agent = "[searchCost " + std::to_string(rand()) + "] ";

        glm::vec2 zero {0, 0};
        double maxDistanceSquared = state->_config._table.diagonalLengthSquared;
        double totalDistanceCost = 0.0;
        double totalAngleCost = 0.0;
        double totalIndirectionCost = 0.0;

        const PhysicalEvent* previousEvent = nullptr;
        for (int eventIndex = 0; eventIndex < events.size(); eventIndex++) {

            auto& event = events[eventIndex];

            if (previousEvent) {
                glm::vec2 way = previousEvent->_targetPosition - event._targetPosition;
                double distanceCost = glm::dot(way, way) / maxDistanceSquared;

                double parentWeight = 1.0;

                if (parentSearchNode && !parentSearchNode->_events.empty()) {
                    auto lastEvent = getLastEvent(parentSearchNode);
                    auto secondLastEvent = getSecondLastEvent(parentSearchNode);
                    assert(lastEvent);
                    assert(secondLastEvent);
                    glm::vec2 targetBallTargetVector = secondLastEvent->_targetPosition - lastEvent->_targetPosition;
                    assert(targetBallTargetVector != zero);
                    float parentDistanceCost = glm::dot(targetBallTargetVector, targetBallTargetVector) / maxDistanceSquared;
                    parentWeight = parentDistanceCost;
                }

                DEBUG(agent << "Distance "
                            << " distanceCost=" << std::to_string(distanceCost) << " "
                            << " parentWeight=" << std::to_string(parentWeight) << " "
                            << " distanceCost * parentWeight=" << std::to_string(distanceCost * parentWeight) << " "
                            << std::endl);

                if (ball._type == "WHITE") {
                    totalDistanceCost += distanceCost * parentWeight;
                } else {
                    totalDistanceCost += distanceCost;
                }
            }
            previousEvent = &event;

            if (event._type == PhysicalEventType::BALL_KICK) {
                continue;
            }

            // There has to be an event before the pocket collision / rail collision / ball collision
            assert(eventIndex+1 < events.size());
            const glm::vec2& previousPosition = events[eventIndex+1]._targetPosition;
            const glm::vec2& currentPosition = event._targetPosition;

            glm::vec2 targetDirection = glm::normalize(currentPosition - previousPosition);

            double cosTheta = 0.0;

            std::stringstream angleDebugOutput {};

            if (event._type == PhysicalEventType::POCKET_COLLISION) {

                auto& pocketId = event.id;
                auto pocket = getPocketById(state, pocketId);
                assert(pocket);

                glm::vec2& pocketNormal = pocket->_normal;
                assert(pocketNormal != zero);

                cosTheta = glm::dot(pocketNormal, -targetDirection);

#ifdef BILLIARD_DEBUG
                angleDebugOutput << "Type=POCKET_COLLISION" << " "
                                 << "pocketNormal=" << pocketNormal << " ";
#endif

            } else if (event._type == PhysicalEventType::RAIL_COLLISION) {

                auto& railId = event.id;
                auto rail = getRailById(state, railId);
                assert(rail);

                glm::vec2& railNormal = rail->_normal;
                assert(railNormal != zero);

                cosTheta = glm::dot(railNormal, -targetDirection);

#ifdef BILLIARD_DEBUG
                angleDebugOutput << "Type=RAIL_COLLISION" << " "
                                 << "railNormal=" << railNormal << " ";
#endif

            } else if (event._type == PhysicalEventType::BALL_COLLISION) {

                assert(parentSearchNode);
                auto lastEvent = getLastEvent(parentSearchNode);
                auto secondLastEvent = getSecondLastEvent(parentSearchNode);
                assert(lastEvent);
                assert(secondLastEvent);
                glm::vec2 targetBallTargetVector = secondLastEvent->_targetPosition - lastEvent->_targetPosition;
                assert(targetBallTargetVector != zero);
                glm::vec2 targetBallTargetDirection = glm::normalize(targetBallTargetVector);

                cosTheta = glm::dot(targetBallTargetDirection, targetDirection);

#ifdef BILLIARD_DEBUG
                angleDebugOutput << "Type=BALL_COLLISION" << " "
                                 << "targetBallTargetDirection=" << targetBallTargetDirection << " ";
#endif
            }

            assert(cosTheta >= 0);
            // cosTheta is 0 for 90 degrees, 1 for 0 degrees -> flip that using a linear function y = -1x + 1
            double angleCost = -1 * cosTheta + 1;
            // y = ax^2 + bx + c
            // P0 = (0, 0)
            // P1 = (1, 1)
            // -> c = 0
            //    1 = a + b
            //    a = 1 - b

            // y = c(e^(ax) + b)
            // P0 = (0, 0)
            // P1 = (1, 1)
            // P2 = (0.8, 0.2)
            // ->
            //    b = -1
            //    1.2 = ce^(0.8a)
            //    ln(1.2) = ln(c) + 0.8a
            //    a = (ln(1.2) - ln(c))/0.8
            //    1 = c(e^a - 1)
            //    c = 1/(e^a - 1)

//            double weightedAngleCost = 1.0/(200.0 - 1.0) * (pow(200.0, angleCost) - 1.0);
//            double weightedAngleCost = 0.0;
//            if (angleCost < 0.8) {
//                // Line between (0,0) and (0.75, 0.2): y = (0.2/0.75)x
//                weightedAngleCost = 0.2/0.75 * angleCost;
//            } else {
//                // Line between (0.75, 0.2) and (1,1): y = ((1-0.2)/(1-0.75))x
//                weightedAngleCost = 0.8/0.25 * angleCost;
//            }

            glm::vec4 t {angleCost*angleCost*angleCost, angleCost*angleCost, angleCost, 1};
            glm::mat4x4 coeff { glm::vec4 {-1, 3, -3, 1}, glm::vec4 {3, -6, 3, 0}, glm::vec4 {-3, 3, 0, 0}, glm::vec4 {1, 0, 0, 0} };
            glm::vec4 x {0, 1, 0.5, 1};
            glm::vec4 y {0, 0, 1, 1};
//            glm::vec2 p0 {0, 0};
//            glm::vec2 p1 {1, 0};
//            glm::vec2 p2 {0.5, 1};
//            glm::vec2 p3 {1, 1};
//            glm::mat4x2 points { p0, p1, p2, p3 };
            glm::mat2x4 points { x, y };
            auto coeffPoints = coeff * points;
            glm::vec2 result = t * coeff * points;
            double weightedAngleCost = result.y;

//            totalAngleCost += angleCost;
            totalAngleCost += weightedAngleCost;

            std::stringstream indirectionDebugOutput {};

            if (event._type == PhysicalEventType::BALL_COLLISION) {
                double step = 1.0 / ((double) SEARCH_MAX_BALL_COLLISIONS);
                totalIndirectionCost += step;

#ifdef BILLIARD_DEBUG
                indirectionDebugOutput << " Ball collision -> indirection +" << std::to_string(step) << " ";
#endif
            }

            DEBUG(agent << "Event " << std::to_string(eventIndex) << " "
                           << "previousPosition=" << previousPosition << " "
                           << "currentPosition=" << currentPosition << " "
                           << "targetDirection=" << targetDirection << " "
                           << std::endl);
            DEBUG(agent << "Event " << std::to_string(eventIndex) << " "
                        << angleDebugOutput.str()
                        << "cosTheta=" << std::to_string(cosTheta) << " "
                        << "angleCost=" << std::to_string(angleCost) << " "
                        << "weightedAngleCost=" << std::to_string(weightedAngleCost) << " "
                        << std::endl);
            DEBUG(agent << "Event " << std::to_string(eventIndex) << " "
                        << "t=" << glm::to_string(t) << " "
                        << "points=" << glm::to_string(points) << " "
                        << "coeff=" << glm::to_string(coeff) << " "
                        << "result=" << glm::to_string(result) << " "
                        << "coeff * points=" << glm::to_string(coeffPoints) << " "
                        << std::endl);
            DEBUG(agent << "Event " << std::to_string(eventIndex) << " "
                        << "Indirection: " << indirectionDebugOutput.str()
                        << std::endl);
        }

        double totalCost = totalDistanceCost + totalAngleCost + totalIndirectionCost;
        uint64_t totalCostInt = totalCost * COST_FLOAT_TO_INT_FACTOR;

        DEBUG(agent << "Result: "
                    << "totalDistanceCost=" << std::to_string(totalDistanceCost) << " "
                    << "totalAngleCost=" << std::to_string(totalAngleCost) << " "
                    << "totalIndirectionCost=" << std::to_string(totalIndirectionCost) << " "
                    << "totalCost=" << std::to_string(totalCost) << " "
                    << "totalCostInt=" << std::to_string(totalCostInt) << " "
                    << std::endl);

        // TODO: Improve heuristic
        return totalCostInt; // TODO: tweak number
    }

    ///////////////////////////////////////////////////////////////////////
    //// SIMULATION
    ///////////////////////////////////////////////////////////////////////

    std::optional<event::Event> nextEvent(const node::System& system, const std::shared_ptr<SearchState>& state);
    node::Layer createLayer(const event::Event& event, const node::Layer& previousLayer, const std::shared_ptr<SearchState>& state);
    std::vector<std::string> getTypeOf(const Search& search, const std::unordered_map<std::string, Ball>& state);
    uint64_t simulationCost(const node::System& system, uint64_t searchCost, const std::shared_ptr<SearchState>& state);

    std::vector<std::shared_ptr<SearchNode>>
    simulate(const std::shared_ptr<SearchNode>& input, const std::shared_ptr<SearchState>& state, bool mustBeValid) {
        auto parentSearchInput = input->_parent->asSearch();
        auto simulationInput = input->asSimulation();
        auto& system = simulationInput->_simulation;
        std::vector<std::shared_ptr<SearchNode>> expanded;

        std::string agent = "[simulate " + std::to_string(system._id) + "] ";

        DEBUG(agent << "Start simulation" << std::endl);
        int eventCount = 0;

        while(!system.final()) {
            auto event = nextEvent(system, state);

            if (event) {
                DEBUG(agent << "nextEvent: " << event << std::endl);
                eventCount++;
                // TODO: uncomment
//                if (eventCount > MAX_EVENTS) {
//                    DEBUG(agent << "Too many events. Cancel simulation." << std::endl);
//                    break;
//                }
                if (eventCount == MAX_EVENTS) {
                    DEBUG(agent << "Too many events. Cancel simulation." << std::endl);
                }
                if (eventCount > 100) { // TODO: remove this
                    DEBUG(agent << "REALLY Too many events. Cancel simulation." << std::endl);
                    break;
                }
                auto layer = createLayer(*event, system.lastLayer(), state);
                system.append(layer);

                DEBUG(agent << "Layer " << layer << std::endl);
#ifdef BILLIARD_DEBUG
                logBalls(layer.dynamicBalls());
#endif

                if (layer.final()) {
                    auto searchedTypes = getTypeOf(parentSearchInput->_search,
                                                parentSearchInput->_state);
                    if (mustBeValid && !state->_config._rules._isValidEndState(searchedTypes, layer)) {
                        DEBUG(agent << "End state is not valid" << std::endl);
                        return expanded;
                    }

                    int simCount = 0;
                    auto path = input->path();
                    for(auto pathNode : path) {
                        if (pathNode->_type == SearchNodeType::SIMULATION) {
                            simCount++;
                        }
                    }

                    auto cost = input->_cost + simulationCost(simulationInput->_simulation, input->_searchCost, state);

                    if (simCount < BREAKS) {
                        DEBUG(agent << "Prepare simulated output for next break" << std::endl);
                        auto search = state->_config._rules._nextSearch(parentSearchInput->_search, searchedTypes);
                        auto modifiedLayer = state->_config._rules._modifyState(layer);

                        std::vector<Ball> newBallPositions;
                        bool hasTypeToSearch = false;
                        for (auto& node : modifiedLayer._nodes) {
                            auto inRest = node.second.toInRest();
                            if (inRest) {
                                newBallPositions.emplace_back(Ball{
                                        inRest->_ball._position,
                                        parentSearchInput->_state.at(node.first)._type,
                                        node.first});
                                hasTypeToSearch |= std::count(search._types.begin(), search._types.end(),
                                                              parentSearchInput->_state.at(node.first)._type) > 0;
                            }
                        }

                        if (hasTypeToSearch) {
                            auto enrichedWithPockets = addPocketsAsInitialTargets(State{newBallPositions}, search, state->_config);
                            for (auto& output : enrichedWithPockets) {
                                output->_parent = input;
                                output->_cost = cost;
                                output->_searchCost = 0;
                            }
                            expanded.insert(expanded.end(), enrichedWithPockets.begin(), enrichedWithPockets.end());
                        } else {
                            DEBUG(agent << "Solution found with cost " << cost << std::endl);
                            input->_isSolution = true;
                            input->_cost = cost;
                            expanded.emplace_back(input);
                        }
                    } else {
                        DEBUG(agent << "Solution found with cost " << cost << std::endl);
                        input->_isSolution = true;
                        input->_cost = cost;
                        expanded.emplace_back(input);
                    }
                }
            } else {
                DEBUG(agent << "No event found! Simulation discarded");
                break;
            }
        }

        return expanded;
    }

    uint64_t simulationCost(const node::System& system, uint64_t searchCost, const std::shared_ptr<SearchState>& state) {

        std::string agent = "[simulationCost " + std::to_string(system._id) + "] ";

        double totalScore = 0.0;
        for (auto& node : system.lastLayer()._nodes) {
            if (node.second._type == billiard::search::node::NodeType::BALL_POTTING) {
                auto& ballType = node.second._ballType;
                double score = state->_config._rules._scoreForPottedBall(ballType);
                DEBUG(agent << "Potted "
                            << node.first << " "
                            << "(" << ballType << ")" << " "
                            << "scoring: " << std::to_string(score) << " "
                            << std::endl);
                totalScore += score;
            }
        }

        double totalCost = 1.0 - std::min(totalScore, 1.0);
        uint64_t totalCostInt = totalCost * COST_FLOAT_TO_INT_FACTOR; // TODO: remove?

        double simulationCostCap = SIMULATION_COST_FRACTION_OF_SEARCH_COST * (double) searchCost;
        uint64_t weightedTotalCostInt = totalCost * simulationCostCap;

        DEBUG(agent << "Result: "
                    << "totalScore=" << std::to_string(totalScore) << " "
                    << "totalCost=" << std::to_string(totalCost) << " "
                    << "totalCostInt=" << std::to_string(totalCostInt) << " "
                    << "searchCost=" << std::to_string(searchCost) << " "
                    << std::endl);
        DEBUG(agent << "Result: "
                    << "simulationWeight=" << std::to_string(SIMULATION_COST_FRACTION_OF_SEARCH_COST) << " "
                    << "simulationCostCap=" << std::to_string(simulationCostCap) << " "
                    << "weightedTotalCostInt=" << std::to_string(weightedTotalCostInt) << " "
                    << std::endl);

        // TODO: Improve heuristic
        return weightedTotalCostInt;
    }

    std::optional<event::Event> nextBallInRest(const std::pair<std::string, node::Node>& ball);
    std::optional<event::Event> nextRolling(const std::pair<std::string, node::Node>& ball,
                                            const std::unordered_map<std::string, float>& nextRollingEvents);
    std::optional<event::Event>
    nextBallCollision(const std::pair<std::string, node::Node>& ball,
                      const std::unordered_map<std::string, node::Node>& balls,
                      float diameter);
    std::optional<event::Event>
    nextRailCollision(const std::pair<std::string, node::Node>& ball, const std::vector<Rail>& rails,
        const std::unordered_map<std::string, std::string>& lastRailCollisions);
    std::optional<event::Event>
    nextPocketCollision(const std::pair<std::string, node::Node>& ball, const std::vector<Pocket>& pockets);
    std::optional<event::Event> min(const std::optional<event::Event>& event,
                                    const std::optional<event::Event>& currentEvent);

    std::optional<event::Event> nextEvent(const node::System& system, const std::shared_ptr<SearchState>& state) {

        std::optional<event::Event> nextEvent = std::nullopt;

        auto layer = system.lastLayer();
        for (auto& ball : layer.dynamicBalls()) {

            // läuft Kugel nächstens aus?
            nextEvent = min(nextBallInRest(ball), nextEvent);
            // Beginnt die Kugel zu rollen?
            nextEvent = min(nextRolling(ball, system._nextRolling), nextEvent);
            // Kollision mit statischer Kugelr
            nextEvent = min(nextBallCollision(ball, layer.staticBalls(), state->_config._ball._diameter), nextEvent);
            // Kollision mit anderer dynamischer Kugel
            nextEvent = min(nextBallCollision(ball, layer.dynamicBalls(), state->_config._ball._diameter), nextEvent);
            // Kollision mit Bande
            nextEvent = min(nextRailCollision(ball, state->_config._table._rails, system._lastRailCollisions),
                            nextEvent);
            // Kollision mit Zielloch
            nextEvent = min(nextPocketCollision(ball, state->_config._table._pockets), nextEvent);
        }

        return nextEvent ? std::make_optional<event::Event>(*nextEvent) : std::nullopt;
    }

    std::optional<event::Event> nextRolling(const std::pair<std::string, node::Node>& ball,
                                            const std::unordered_map<std::string, float>& nextRollingEvents) {
        if (nextRollingEvents.find(ball.first) != nextRollingEvents.end()) {
            event::Event event {
                event::EventType::BALL_ROLLING,
                nextRollingEvents.find(ball.first)->second,
                event::EventVariant{event::BallRolling{ball.first}}};
            DEBUG("[nextBallRolling]: " << event << std::endl);
            return event;
        }
        return std::nullopt;
    }

    std::optional<event::Event> nextBallInRest(const std::pair<std::string, node::Node>& ball) {
        auto state = ball.second.after();
        if (state) {
            event::Event event {
                    event::EventType::BALL_IN_REST,
                    billiard::physics::timeToStop(state->_acceleration, state->_velocity),
                    event::EventVariant{event::BallInRest{ball.first}}};
            DEBUG("[nextBallInRest]: " << event << std::endl);
            return event;
        }
        return std::nullopt;
    }

    std::optional<event::Event>
    nextBallCollision(const std::pair<std::string, node::Node>& ball,
                      const std::unordered_map<std::string, node::Node>& balls,
                      const float diameter) {
        std::optional<event::Event> nextEvent = std::nullopt;

        auto state = ball.second.after();
        if (state) {
            for (auto& otherBall : balls) {
                if (otherBall.first != ball.first) {
                    auto otherState = otherBall.second.after();
                    if (otherState) {
                        auto collisionTime = billiard::physics::timeToCollision(
                                state->_acceleration, state->_velocity, state->_position,
                                otherState->_acceleration, otherState->_velocity, otherState->_position,
                                diameter
                                );

                        if (collisionTime) {
                            event::Event event {
                                event::EventType::BALL_COLLISION,
                                *collisionTime,
                                event::EventVariant { event::BallCollision{ ball.first, otherBall.first } }
                            };
                            nextEvent = min(event, nextEvent);
                        }
                    }

                }
            }
        }
        DEBUG("[nextBallCollision]: " << nextEvent << std::endl);

        return nextEvent;
    }

    std::optional<event::Event> nextRailCollision(const std::pair<std::string, node::Node>& ball,
                                                  const std::vector<Rail>& rails,
                                                  const std::unordered_map<std::string, std::string>& lastRailCollisions) {

        std::optional<event::Event> nextEvent = std::nullopt;

        auto state = ball.second.after();
        if (state) {

            auto& ballPosition = state->_position;
            auto& ballVelocity = state->_velocity;
            auto& ballAcceleration = state->_acceleration;

            auto previousRailId = lastRailCollisions.find(ball.first) != lastRailCollisions.end() ?
                    lastRailCollisions.at(ball.first) :
                    "";

            for (auto& rail : rails) {

                if (previousRailId == rail._id) {
                    continue;
                }

                auto& railPoint1 = rail._start;
                auto& railPoint2 = rail._end;
                auto& shiftedRailPoint1 = rail._shiftedStart;
                auto& shiftedRailPoint2 = rail._shiftedEnd;
                auto collisionTime = billiard::physics::timeToCollisionWithRail(ballPosition, ballVelocity, ballAcceleration, railPoint1, railPoint2, shiftedRailPoint1, shiftedRailPoint2);
                if (collisionTime) {
                    event::Event event {
                            event::EventType::BALL_RAIL_COLLISION,
                            *collisionTime,
                            event::EventVariant { event::BallRailCollision{ ball.first, rail._id } }
                    };
                    nextEvent = min(event, nextEvent);
                }
            }
        }

        DEBUG("[nextRailCollision]: " << nextEvent << std::endl);

        return nextEvent;
    }

    std::optional<event::Event> nextPocketCollision(const std::pair<std::string, node::Node>& ball,
                                                    const std::vector<Pocket>& pockets) {
        std::optional<event::Event> nextEvent = std::nullopt;
        auto state = ball.second.after();

        if (state) {
            for (auto& pocket : pockets) {
                auto time = billiard::physics::timeToTarget(
                        pocket._position,
                        state->_position,
                        state->_velocityNormed,
                        state->_velocity,
                        state->_acceleration,
                        pocket._radius
                        );

                if (time) {
                    event::Event event {
                            event::EventType::BALL_POTTING,
                            *time,
                            event::EventVariant { event::BallPotting{ ball.first, pocket._id } }
                    };
                    nextEvent = min(event, nextEvent);
                }
            }
        }

        DEBUG("[nextPocketCollision]: " << nextEvent << std::endl);

        return nextEvent;
    }

    node::Node createPottingNode(const event::BallPotting& event,
                                 float time,
                                 const node::Node& previousNode,
                                 const std::shared_ptr<SearchState>& state) {
        auto previousState = *previousNode.after();
        auto position = billiard::physics::position(previousState._acceleration,
                                                    previousState._velocity,
                                                    time,
                                                    previousState._position);
        auto velocityAtPotting = billiard::physics::accelerate(previousState._acceleration,
                                                               previousState._velocity,
                                                               time);
        state::BallState newState1 {
                position,
                velocityAtPotting,
                state->_ball._accelerationLength,
                state->_ball._slideAccelerationLength,
                previousState._isRolling
        };

        state::BallState newState2 {
                position,
                glm::vec2{0, 0},
                state->_ball._accelerationLength,
                state->_ball._slideAccelerationLength,
                false
        };
        node::BallPottingNode pottingNode {newState1, newState2, event};
        return node::Node{node::NodeType::BALL_POTTING, previousNode._ballType, node::NodeVariant(pottingNode)};
    }

    node::Node createRailCollisionNode(const event::BallRailCollision& event,
                                 float time,
                                 const node::Node& previousNode,
                                 const std::shared_ptr<SearchState>& state) {
        auto previousState = *previousNode.after();
        auto position = billiard::physics::position(previousState._acceleration,
                                                    previousState._velocity,
                                                    time,
                                                    previousState._position);
        auto velocityBeforeCollision = billiard::physics::accelerate(previousState._acceleration,
                                                                      previousState._velocity,
                                                                      time);

        glm::vec2 normal;
        for (auto& rail : state->_config._table._rails) { // TODO: Hold rail as map
            if (rail._id == event._rail) {
                normal = rail._normal;
            }
        }

        auto velocityAfterCollision = billiard::physics::railCollision(velocityBeforeCollision, normal);

        state::BallState newState1 {
                position,
                velocityBeforeCollision,
                state->_ball._accelerationLength,
                state->_ball._slideAccelerationLength,
                previousNode.before()->_isRolling
        };

        state::BallState newState2 {
                position,
                velocityAfterCollision,
                state->_ball._accelerationLength,
                state->_ball._slideAccelerationLength,
                true
        };
        node::BallRailCollisionNode collisionNode {newState1, newState2, event};
        return node::Node{node::NodeType::BALL_RAIL_COLLISION, previousNode._ballType, node::NodeVariant(collisionNode)};
    }

    node::Node createInRestNode(const event::BallInRest& event,
                                 float time,
                                 const node::Node& previousNode,
                                 const std::shared_ptr<SearchState>& state) {
        auto previousState = *previousNode.after();
        auto position = billiard::physics::position(previousState._acceleration,
                                                    previousState._velocity,
                                                    time,
                                                    previousState._position);
        state::BallState newState {
                position,
                glm::vec2{0, 0},
                state->_ball._accelerationLength,
                state->_ball._slideAccelerationLength,
                false
        };
        node::BallInRestNode inRestNode {newState};
        return node::Node{node::NodeType::BALL_IN_REST, previousNode._ballType, node::NodeVariant(inRestNode)};
    }

    node::Node createRollingNode(const event::BallRolling& event,
                                float time,
                                const node::Node& previousNode,
                                const std::shared_ptr<SearchState>& state) {
        auto previousState = *previousNode.after();
        auto position = billiard::physics::position(previousState._acceleration,
                                                    previousState._velocity,
                                                    time,
                                                    previousState._position);
        auto velocity = billiard::physics::accelerate(previousState._acceleration, previousState._velocity, time);

        state::BallState beforeState {
                position,
                velocity,
                state->_ball._accelerationLength,
                state->_ball._slideAccelerationLength,
                false
        };

        state::BallState afterState {
                position,
                velocity,
                state->_ball._accelerationLength,
                state->_ball._slideAccelerationLength,
                true
        };
        node::BallMovingNode rollingNode {beforeState, afterState};
        return node::Node{node::NodeType::BALL_MOVING, previousNode._ballType, node::NodeVariant(rollingNode)};
    }

    node::Node createBallCollisionNode(const event::BallCollision& event,
                                       const glm::vec2& position,
                                       const glm::vec2& velocityBefore,
                                       const glm::vec2& velocityAfter,
                                       const std::string& ballType,
                                       const state::BallState& previousState,
                                       const std::shared_ptr<SearchState>& state) {
        state::BallState newState1 {
                position,
                velocityBefore,
                state->_ball._accelerationLength,
                state->_ball._slideAccelerationLength,
                previousState._isRolling
        };
        state::BallState newState2 {
                position,
                velocityAfter,
                state->_ball._accelerationLength,
                state->_ball._slideAccelerationLength,
                previousState._isRolling
        };
        node::BallCollisionNode collisionNode {newState1, newState2, event};
        return node::Node{node::NodeType::BALL_COLLISION, ballType, node::NodeVariant(collisionNode)};
    }

    std::unordered_map<std::string, node::Node> createBallCollisionNodes(
                                 const event::BallCollision& event,
                                 float time,
                                 const node::Node& previousNode1,
                                 const node::Node& previousNode2,
                                 const std::shared_ptr<SearchState>& state) {

        auto previousState1 = previousNode1.after();
        auto previousState2 = previousNode2.after();

        auto positionBeforeCollision1 = billiard::physics::position(previousState1->_acceleration,
                                                                    previousState1->_velocity,
                                                                    time,
                                                                    previousState1->_position);
        auto velocityBeforeCollision1 = billiard::physics::accelerate(previousState1->_acceleration,
                                                                      previousState1->_velocity,
                                                                      time);
        auto positionBeforeCollision2 = billiard::physics::position(previousState2->_acceleration,
                                                                    previousState2->_velocity,
                                                                    time,
                                                                    previousState2->_position);
        auto velocityBeforeCollision2 = billiard::physics::accelerate(previousState2->_acceleration,
                                                                      previousState2->_velocity,
                                                                      time);
        auto collisions = billiard::physics::elasticCollision(positionBeforeCollision1,
                                                              velocityBeforeCollision1,
                                                              positionBeforeCollision2,
                                                              velocityBeforeCollision2,
                                                              previousState1->_isRolling,
                                                              previousState2->_isRolling);

        return std::unordered_map<std::string, node::Node> {
                {event._ball1, createBallCollisionNode(event, positionBeforeCollision1, velocityBeforeCollision1,
                                                       collisions.first, previousNode1._ballType, *previousState1,
                                                       state)},
                {event._ball2, createBallCollisionNode(event, positionBeforeCollision2, velocityBeforeCollision2,
                                                       collisions.second, previousNode2._ballType, *previousState2,
                                                       state)},
        };
    }

    std::unordered_map<std::string, node::Node> createNodes(const event::Event& event,
                                                           const std::unordered_map<std::string, node::Node>& previousNodes,
                                                           const std::shared_ptr<SearchState>& state) {
        auto asPotting = event.toPotting();
        if (asPotting) {
            auto& previousNode = previousNodes.at(asPotting->_ball);

            return std::unordered_map<std::string, node::Node>{
                    {asPotting->_ball, createPottingNode(*asPotting, event._time, previousNode, state)}
            };
        }

        auto asBallCollision = event.toBallCollision();
        if (asBallCollision) {
            auto& previousNode1 = previousNodes.at(asBallCollision->_ball1);
            auto& previousNode2 = previousNodes.at(asBallCollision->_ball2);

            return createBallCollisionNodes(*asBallCollision, event._time, previousNode1, previousNode2, state);
        }

        auto asRailCollision = event.toRailCollision();
        if (asRailCollision) {
            auto& previousNode = previousNodes.at(asRailCollision->_ball);

            return std::unordered_map<std::string, node::Node>{
                    {asRailCollision->_ball, createRailCollisionNode(*asRailCollision, event._time, previousNode, state)}
            };
        }

        auto asInRest = event.toInRest();
        if (asInRest) {
            auto& previousNode = previousNodes.at(asInRest->_ball);

            return std::unordered_map<std::string, node::Node>{
                    {asInRest->_ball, createInRestNode(*asInRest, event._time, previousNode, state)}
            };
        }

        auto asRolling = event.toRolling();
        if (asRolling) {
            auto& previousNode = previousNodes.at(asRolling->_ball);

            return std::unordered_map<std::string, node::Node>{
                    {asRolling->_ball, createRollingNode(*asRolling, event._time, previousNode, state)}
            };
        }

        return std::unordered_map<std::string, node::Node>{};
    }

    node::Layer createLayer(const event::Event& event, const node::Layer& previousLayer,
                            const std::shared_ptr<SearchState>& state) {
        std::unordered_map<std::string, node::Node> nodes;

        auto affected = event.affected();

        for (auto& previousNode : previousLayer._nodes) {
            if (affected.find(previousNode.first) == affected.end()) {
                if (previousNode.second.isStatic()) {
                    nodes.insert(previousNode);
                } else {
                    auto ballState = previousNode.second.after();
                    auto velocity = billiard::physics::accelerate(ballState->_acceleration, ballState->_velocity, event._time);
                    auto position = billiard::physics::position(ballState->_acceleration,
                                                                ballState->_velocity,
                                                                event._time,
                                                                ballState->_position);
                    state::BallState newState {
                            position,
                            velocity,
                            state->_ball._accelerationLength,
                            state->_ball._slideAccelerationLength,
                            ballState->_isRolling
                    };
                    node::BallMovingNode deltaNode {newState, newState};
                    node::Node result{node::NodeType::BALL_MOVING, previousNode.second._ballType, node::NodeVariant(deltaNode)};
                    nodes.insert({previousNode.first, result});
                }
            }
        }

        auto nodesFromEvent = createNodes(event, previousLayer._nodes, state);
        nodes.insert(nodesFromEvent.begin(), nodesFromEvent.end());

        return node::Layer{previousLayer._time + event._time, nodes};
    }

    std::vector<std::string> getTypeOf(const Search& search, const std::unordered_map<std::string, Ball>& state) {
        if (!search._types.empty()) {
            return search._types;
        }

        return state.count(search._id) ? std::vector<std::string>{state.at(search._id)._type} : std::vector<std::string>{};
    }

    std::optional<event::Event> min(const std::optional<event::Event>& event,
                                    const std::optional<event::Event>& currentEvent) {

        if (event.has_value() && currentEvent.has_value()) {
            return event->_time < currentEvent->_time ? event : currentEvent;
        } else if (event.has_value()) {
            return event;
        } else if (currentEvent.has_value()) {
            return currentEvent;
        }
        return std::nullopt;
    }

#ifdef BILLIARD_DEBUG
    std::string readable(const PhysicalEventType& eventType) {
        switch (eventType) {
            case PhysicalEventType::BALL_COLLISION:
                return "BALL_COLLISION";
            case PhysicalEventType::RAIL_COLLISION:
                return "RAIL_COLLISION";
            case PhysicalEventType::POCKET_COLLISION:
                return "POCKET_COLLISION";
            case PhysicalEventType::BALL_KICK:
                return "BALL_KICK";
        }

        return "NO PHYSICAL EVENT TYPE FOUND";
    }

    std::string readable(const event::EventType& eventType) {
        switch (eventType) {
            case event::EventType::BALL_COLLISION:
                return "BALL_COLLISION";
            case event::EventType::BALL_ROLLING:
                return "BALL_ROLLING";
            case event::EventType::BALL_RAIL_COLLISION:
                return "BALL_RAIL_COLLISION";
            case event::EventType::BALL_POTTING:
                return "BALL_POTTING";
            case event::EventType::BALL_IN_REST:
                return "BALL_IN_REST";
        }

        return "NO EVENT TYPE FOUND";
    }

    std::string readable(const node::NodeType& nodeType) {
        switch (nodeType) {
            case node::NodeType::BALL_MOVING:
                return "BALL_MOVING";
            case node::NodeType::BALL_COLLISION:
                return "BALL_COLLISION";
            case node::NodeType::BALL_RAIL_COLLISION:
                return "BALL_RAIL_COLLISION";
            case node::NodeType::BALL_POTTING:
                return "BALL_POTTING";
            case node::NodeType::BALL_SHOT:
                return "BALL_SHOT";
            case node::NodeType::BALL_IN_REST:
                return "BALL_IN_REST";
        }
        return "NO NODE TYPE FOUND";
    }

    std::string readable(const SearchActionType& actionType) {
        switch (actionType) {
            case SearchActionType::DIRECT:
                return "DIRECT";
            case SearchActionType::RAIL:
                return "RAIL";
            case SearchActionType::NONE:
                return "NONE";
        }
        return "NO ACTION TYPE FOUND";
    }

    void logBalls(const std::unordered_map<std::string, node::Node>& balls) {
        for(auto& ball : balls) {
            DEBUG("[simulate] dynamic ball: " << ball.first << " " << ball.second << std::endl);
        }
    }

    std::ostream& operator<<(std::ostream& os, const glm::vec2& vector) {
        os << "(" << vector.x << ", " << vector.y << ")";
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const glm::vec4& vector) {
        os << "(" << vector.x << ", " << vector.y << ", " << vector.z << ", " << vector.w << ")";
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const PhysicalEvent& event) {
        os << "PhysicalEvent { "
           << "type=" << readable(event._type) << " "
           << "targetPosition=" << event._targetPosition
           << " }";
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const std::vector<PhysicalEvent>& events) {
        os << "[ ";
        for (auto& event : events) {
            os << event << " ";
        }
        os << "]";
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const state::BallState& ball){
        os << "BallState { "
           << "position=" << ball._position << " "
           << "velocity=" << ball._velocity << " "
           << "acceleration=" << ball._acceleration << " "
           << "isRolling=" << (ball._isRolling ? "true" : "false") <<
           " }";
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const node::Node& node){
        auto beforeState = node.before();
        assert(beforeState);
        auto afterState = node.after();
        assert(afterState);
        os << "Node { "
           << "type=" << readable(node._type) << " "
           << "balltype=" << node._ballType << " "
           << "before= " << *beforeState << " "
           << "after= " << *afterState
           << " }";
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const std::set<std::string>& set) {
        os << "Set { ";
        uint32_t index = 0;
        for(auto& value : set) {
            os << value;
            if (index < set.size() - 1) {
                os << ", ";
            }
            index++;
        }
        os << " }";
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const std::optional<event::Event>& event) {
        if (event) {
            os << "Event { "
               << "time=" << event->_time << " "
               << "type=" << readable(event->_type) << " "
               << "affected=" << event->affected()
               << " }";
        } else {
            os << "Event { " << "none" << " }";
        }
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const node::Layer& layer) {
        os << "Layer { "
           << "time=" << layer._time << " "
           << "first=" << layer._isFirst << " "
           << "last=" << layer._isLast << " "
           << "final=" << layer.final()
           << " }";
        return os;
    }

    std::string logPath(const std::vector<const SearchNode *>& path) {
        std::stringstream out {};
        out << "-> ";
        for (const SearchNode* node : path) {
            if (node->_type == SearchNodeType::SEARCH) {
                auto searchNode = node->asSearch();
                auto& id = searchNode->_ballId;
                out << "" << id << " -> ";
            } else {
                out << "-> (SIMULATION) -> ";
            }
        }
        return out.str();
    }
#endif
}

