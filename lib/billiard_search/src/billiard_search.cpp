#include <billiard_search/billiard_search.hpp>
#include <billiard_physics/billiard_physics.hpp>
#include <billiard_debug/billiard_debug.hpp>
#include <process/process.hpp>
#include <memory>
#include <optional>
#include <iostream>

namespace billiard::search {

    #define PROCESSES 3
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

    struct SearchNode;
    std::vector<node::System> mapToSolution(const std::shared_ptr<SearchNode>& solution);
    std::shared_ptr<SearchNode> convertToInput(const State& state, const Search& search);
    std::vector<std::shared_ptr<SearchNode>>
    expand(const std::shared_ptr<SearchNode>& input, const std::shared_ptr<SearchState>& config);
    std::vector<std::shared_ptr<SearchNode>>
    simulate(const std::shared_ptr<SearchNode>& input, const std::shared_ptr<SearchState>& config);
    std::vector<std::shared_ptr<SearchNode>>
    expandSearchNode(const std::shared_ptr<SearchNode>& input, const std::shared_ptr<SearchState>& config);
    std::vector<std::shared_ptr<SearchNode>>
    addPocketsAsInitialTargets(const State& state, const Search& search, const Configuration& config);
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

    return processManager.process(addPocketsAsInitialTargets(state, search, config), solutions,
                                  std::make_shared<SearchState>(SearchState{config}));
}

///////////////////////////////////////////////////////////////////////
//// PRIVATE IMPLEMENTATION
///////////////////////////////////////////////////////////////////////

namespace billiard::search {

    std::vector<node::System> mapToSolution(const std::shared_ptr<SearchNode>& solution) {
        return solution->allSimulations();
    }

    std::vector<std::shared_ptr<SearchNode>>
    expand(const std::shared_ptr<SearchNode>& input, const std::shared_ptr<SearchState>& state) {
        if (input->_type == SearchNodeType::SEARCH) {
            return expandSearchNode(input, state);
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
    prepareForSimulation(const std::shared_ptr<SearchNode>& input, const std::shared_ptr<SearchState>& state);
    std::vector<std::shared_ptr<SearchNode>>
    expandSearchNodeByBanks(const std::shared_ptr<SearchNode>& input, const std::shared_ptr<SearchState>& state);
    std::vector<std::shared_ptr<SearchNode>>
    expandSearchNodeByBank(const std::shared_ptr<SearchNode>& input, const std::shared_ptr<SearchState>& state,
                         const std::shared_ptr<SearchNodeSearch>& searchInput, uint8_t depth);
    uint64_t searchCost(const std::vector<PhysicalEvent>& events);
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

            if ((searchInput->_action != SearchActionType::NONE && ball.second._type == "WHITE") ||
//                (searchInput->_action == SearchActionType::NONE && (ball.first == searchInput->_search._id || ball.second._type == searchInput->_search._type))) {
                ((ball.first == searchInput->_search._id ||
                std::count(searchInput->_search._types.begin(), searchInput->_search._types.end(), ball.second._type)))) {

                auto result = expandSearchNodeByBall(input, state, searchInput, ball);
                expanded.insert(expanded.end(), result.begin(), result.end());
            }
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

            float ballDiameterSquared = state->_config._ball._diameterSquared;
            float squaredDistance = billiard::physics::pointToLineSegmentSquaredDistance(ball._position, targetPosition,
                                                                                         other.second._position);
            if (squaredDistance <= ballDiameterSquared) {
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
        assert(pocket.has_value());

        // TODO: check if that is possible

        // Only possible to hit ball if no other ball is on the way
        if (collidesOnTheWay(parent, state, ball, "", pocket->_position)) {
            return nullptr;
        }

        auto result = SearchNode::search(parentNode);
        auto resultSearchNode = result->asSearch();
        resultSearchNode->_events.push_back(PhysicalEvent { PhysicalEventType::POCKET_COLLISION, pocket->_position });
        resultSearchNode->_events.push_back(PhysicalEvent { PhysicalEventType::BALL_KICK, ball._position });

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
        resultSearchNode->_events.push_back(PhysicalEvent { PhysicalEventType::BALL_COLLISION, targetPosition });
        resultSearchNode->_events.push_back(PhysicalEvent { PhysicalEventType::BALL_KICK, ball._position });

        return result;
    }

    std::shared_ptr<SearchNode> expandBallIfPossible(const std::shared_ptr<SearchNode>& input,
                                                     const std::shared_ptr<SearchState>& state,
                                                     const std::shared_ptr<SearchNodeSearch>& searchInput,
                                                     const std::pair<std::string, Ball>& ball) {
        auto& parent = searchInput;
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
            result->_cost = input->_cost + searchCost(resultSearchNode->_events);

            if (ball.second._type == "WHITE") {
                auto prepared = prepareForSimulation(result, state); // TODO: Comment if search test
                expanded.insert(expanded.end(), prepared.begin(), prepared.end());
                /*result->_isSolution = true;
                expanded.push_back(result);*/
            } else {
                expanded.push_back(result);
            }
        }

        return expanded;
    }

#ifdef BILLIARD_DEBUG
    std::string readable(const PhysicalEventType& eventType);
    void logBalls(const std::unordered_map<std::string, node::Node>& balls);

    std::ostream& operator<<(std::ostream& os, const glm::vec2& vector){
        os << "(" << vector.x << ", " << vector.y << ")";
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const PhysicalEvent& event){
        os << "PhysicalEvent { "
              << "type=" << readable(event._type) << " "
              << "targetPosition=" << event._targetPosition
              << " }";
        return os;
    }
#endif

    glm::vec2
    calculateMinimalVelocity(const std::vector<const SearchNode*>& nodes, const std::shared_ptr<SearchState>& state) {
        static glm::vec2 zero{0, 0};
        float minimalVelocityInPocket = state->_config._table.minimalPocketVelocity; // TODO: find a good number
        glm::vec2 minimalForce { 0, 0 };

        PhysicalEvent* previousEvent = nullptr;

        for (int nodeIndex = nodes.size() - 1; nodeIndex >= 0; nodeIndex--) {

            auto node = nodes[nodeIndex];
            auto searchNode = node->asSearch();

            if (searchNode->_action != SearchActionType::NONE) {

                for (auto& event : searchNode->_events) {

                    if (previousEvent) {

                        DEBUG("[calculateMinimalVelocity] " << "previous: " << *previousEvent << " current: " << event << std::endl);

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
                            minimalForce = billiard::physics::elasticCollisionReverse(minimalForce, originVelocity);

                            // Roll to collision point
                            float distance = glm::length(toTarget);
                            minimalForce = billiard::physics::startVelocity(minimalForce, distance);

                            DEBUG("[calculateMinimalVelocity] " << "Ball collision:"
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
                            minimalForce = billiard::physics::startVelocity(finalVelocity, distance);

                            DEBUG("[calculateMinimalVelocity] " << "Roll into pocket:"
                                    << " distance: " << distance
                                    << " from: " << currentPosition
                                    << " to: " << targetPosition << std::endl);

                        } else {

                            // Roll some distance between two points
                            glm::vec2& currentPosition = event._targetPosition;
                            glm::vec2& targetPosition = previousEvent->_targetPosition;
                            glm::vec2 toTarget = targetPosition - currentPosition;
                            float distance = glm::length(toTarget);
                            minimalForce = billiard::physics::startVelocity(minimalForce, distance);

                            DEBUG("[calculateMinimalVelocity] " << "Roll somewhere:"
                                << " distance: " << distance
                                << " from: " << currentPosition
                                << " to: " << targetPosition << std::endl);
                        }
                    }

                    previousEvent = &event;
                }
            }
        }

        return minimalForce;
    }

    node::Layer toInputLayer(const std::shared_ptr<SearchNode>& input, const glm::vec2 force, float accelerationLength) {
        std::unordered_map<std::string, node::Node> nodes;

        auto asSearch = input->asSearch();

        for(auto& ball : asSearch->_state) {
            if (ball.second._type == "WHITE") {
                node::BallMovingNode initialEnergyNode{
                        state::BallState{
                                ball.second._position,
                                glm::vec2{0, 0},
                                accelerationLength
                        },
                        state::BallState{
                                ball.second._position,
                                force,
                                accelerationLength}
                };
                node::Node result{node::NodeType::BALL_MOVING, ball.second._type, node::NodeVariant(initialEnergyNode)};
                nodes.insert({ball.first, result});
            } else {
                node::BallInRestNode inRestNode{
                        state::BallState{
                                ball.second._position,
                                glm::vec2{0, 0},
                                accelerationLength
                        }
                };
                node::Node result{node::NodeType::BALL_IN_REST, ball.second._type, node::NodeVariant(inRestNode)};
                nodes.insert({ball.first, result});
            }
        }

        return node::Layer{0, nodes};
    }

    std::vector<std::shared_ptr<SearchNode>>
    prepareForSimulation(const std::shared_ptr<SearchNode>& input, const std::shared_ptr<SearchState>& state) {
        std::vector<std::shared_ptr<SearchNode>> expanded;

        static glm::vec2 zero{0, 0};
        auto path = input->currentPath();
        auto minimalVelocity = calculateMinimalVelocity(path, state);
        assert(minimalVelocity != zero);
        auto minimalVelocityNormalized = glm::normalize(minimalVelocity);

        for (int i = 0; i < FORWARD_SEARCHES; ++i) {
            glm::vec2 increasedVelocity = minimalVelocity + (i * VELOCITY_STEP * minimalVelocityNormalized);
            if (glm::dot(increasedVelocity, increasedVelocity) > MAX_VELOCITY_SQUARED) {
                break;
            }

            auto layer = toInputLayer(input, increasedVelocity, state->_ball._accelerationLength);

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
            result->_cost = input->_cost + searchCost(resultSearchNode->_events);

            if (ball.second._type == "WHITE") {
                auto prepared = prepareForSimulation(result, state);
                expanded.insert(expanded.end(), prepared.begin(), prepared.end());
            } else {
                expanded.push_back(result);
            }
        }

        return expanded;
    }

    uint64_t searchCost(const std::vector<PhysicalEvent>& events) {
        return 0; // TODO: Write heuristic
    }

    ///////////////////////////////////////////////////////////////////////
    //// SIMULATION
    ///////////////////////////////////////////////////////////////////////

#ifdef BILLIARD_DEBUG
    std::string readable(const event::EventType& eventType);
    std::string readable(const node::NodeType& nodeType);

    std::ostream& operator<<(std::ostream& os, const state::BallState& ball){
        os << "BallState { "
           << "position=" << ball._position << " "
           << "velocity=" << ball._velocity << " "
           << "acceleration=" << ball._acceleration << " }";
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

    std::ostream& operator<<(std::ostream& os, const std::optional<event::Event>& event){
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

    std::ostream& operator<<(std::ostream& os, const node::Layer& layer){
        os << "Layer { "
        << "time=" << layer._time << " "
        << "first=" << layer._isFirst << " "
        << "last=" << layer._isLast << " "
        << "final=" << layer.final()
        << " }";
        return os;
    }

#endif

    std::optional<event::Event> nextEvent(const node::System& system, const std::shared_ptr<SearchState>& state);
    node::Layer createLayer(const event::Event& event, const node::Layer& previousLayer, const std::shared_ptr<SearchState>& state);
    std::vector<std::string> getTypeOf(const Search& search, const std::unordered_map<std::string, Ball>& state);
    uint64_t simulationCost(const node::System& system);

    std::vector<std::shared_ptr<SearchNode>>
    simulate(const std::shared_ptr<SearchNode>& input, const std::shared_ptr<SearchState>& state) {
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
                if (eventCount > MAX_EVENTS) {
                    DEBUG(agent << "Too many events. Cancel simulation." << std::endl);
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
                    if (!state->_config._rules._isValidEndState(searchedTypes, layer)) { // TODO: search._type to vector of string
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

                    auto cost = simulationCost(simulationInput->_simulation);

                    if (simCount < BREAKS) {
                        DEBUG(agent << "Prepare simulated output for next break" << std::endl);
                        auto search = state->_config._rules._nextSearch(parentSearchInput->_search, searchedTypes);
                        auto modifiedLayer = state->_config._rules._modifyState(layer);

                        std::vector<Ball> newBallPositions;
                        for (auto& node : modifiedLayer._nodes) {
                            auto inRest = node.second.toInRest();
                            if (inRest) {
                                newBallPositions.emplace_back(Ball{
                                        inRest->_ball._position,
                                        parentSearchInput->_state.at(node.first)._type,
                                        node.first});
                            }
                        }

                        auto enrichedWithPockets = addPocketsAsInitialTargets(State{newBallPositions}, search, state->_config);
                        if (!enrichedWithPockets.empty()) {
                            for (auto& output : enrichedWithPockets) {
                                output->_parent = input;
                                output->_cost = input->_cost + cost;
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

    uint64_t simulationCost(const node::System& system) {
        return 0; // TODO: Write heuristic
    }

    std::optional<event::Event> nextBallInRest(const std::pair<std::string, node::Node>& ball);
    std::optional<event::Event>
    nextBallCollision(const std::pair<std::string, node::Node>& ball,
                      const std::unordered_map<std::string, node::Node>& balls,
                      float diameter, float radius);
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
            // Kollision mit statischer Kugelr
            nextEvent = min(nextBallCollision(ball, layer.staticBalls(), state->_config._ball._diameter,
                                              state->_config._ball._radius), nextEvent);
            // Kollision mit anderer dynamischer Kugel
            nextEvent = min(nextBallCollision(ball, layer.dynamicBalls(), state->_config._ball._diameter,
                                              state->_config._ball._radius), nextEvent);
            // Kollision mit Bande
            nextEvent = min(nextRailCollision(ball, state->_config._table._rails, system._lastRailCollisions),
                            nextEvent);
            // Kollision mit Zielloch
            nextEvent = min(nextPocketCollision(ball, state->_config._table._pockets), nextEvent);
        }

        return nextEvent ? std::make_optional<event::Event>(*nextEvent) : std::nullopt;
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
                      const float diameter, const float radius) {
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
                                diameter, radius
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
                state->_ball._accelerationLength
        };

        state::BallState newState2 {
                position,
                glm::vec2{0, 0},
                state->_ball._accelerationLength
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
                state->_ball._accelerationLength
        };

        state::BallState newState2 {
                position,
                velocityAfterCollision,
                state->_ball._accelerationLength
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
                state->_ball._accelerationLength
        };
        node::BallInRestNode inRestNode {newState};
        return node::Node{node::NodeType::BALL_IN_REST, previousNode._ballType, node::NodeVariant(inRestNode)};
    }

    node::Node createBallCollisionNode(const event::BallCollision& event,
                                       const glm::vec2& position,
                                       const glm::vec2& velocityBefore,
                                       const glm::vec2& velocityAfter,
                                       const std::string& ballType,
                                       const std::shared_ptr<SearchState>& state) {
        state::BallState newState1 {
                position,
                velocityBefore,
                state->_ball._accelerationLength
        };
        state::BallState newState2 {
                position,
                velocityAfter,
                state->_ball._accelerationLength
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
                                                              velocityBeforeCollision2);

        return std::unordered_map<std::string, node::Node> {
                {event._ball1, createBallCollisionNode(event, positionBeforeCollision1, velocityBeforeCollision1,
                                                       collisions.first, previousNode1._ballType, state)},
                {event._ball2, createBallCollisionNode(event, positionBeforeCollision2, velocityBeforeCollision2,
                                                       collisions.second, previousNode2._ballType, state)},
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
                            state->_ball._accelerationLength
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

        void logBalls(const std::unordered_map<std::string, node::Node>& balls) {
            for(auto& ball : balls) {
                DEBUG("[simulate] dynamic ball: " << ball.first << " " << ball.second << std::endl);
            }
        }

#endif
}

