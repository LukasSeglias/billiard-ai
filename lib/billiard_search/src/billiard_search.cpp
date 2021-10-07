#include <billiard_search/billiard_search.hpp>
#include <process/process.hpp>
#include <memory>
#include <optional>

namespace billiard::search {

    #define PROCESSES 6
    #define SYNC_PERIOD_MS 5
    #define BREAKS 1
    #define BANK_INDIRECTION 0
    #define FORWARD_SEARCHES 10
    #define MAX_FORCE_TO_ADD 1.0
    #define FORCE_STEP (MAX_FORCE_TO_ADD / FORWARD_SEARCHES)

    struct SearchNode;
    struct SearchState {

        Configuration _config;
    };
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

    return processManager.process(addPocketsAsInitialTargets(state, search, config), solutions,
                                  std::make_shared<SearchState>(SearchState{config}));
}

///////////////////////////////////////////////////////////////////////
//// PRIVATE IMPLEMENTATION
///////////////////////////////////////////////////////////////////////

namespace billiard::search {

    /*pocket = {
            parent: null,
            action: NONE,
            id: null
            events: []
    };
    red = {
            parent: pocket,
            action: DIRECT,
            ballId: red
            events: [{
                type: POCKET_COLLISION,
                    targetPosition: pocket.position
            }]
    };
    lightred = {
            parent: pocket,
            action: DIRECT,
            ballId: red
            events: [{
                type: BALL_COLLISION,
                    targetPosition: 1
            }]
    };
    white = {
            parent: red,
            action: BY_RAIL,
            ballId: white
            events: [
            {
                type: BALL_COLLISION,
                    targetPosition: point 1
            },
            {
                type: RAIL_COLLISION,
                    targetPosition: point 2
            },
            {
                type: BALL_KICK,
                    targetPosition: point 3
            }
            ]
    };*/

    enum PhysicalEventType {
        BALL_COLLISION,
        RAIL_COLLISION,
        POCKET_COLLISION,
        BALL_KICK
    };

    struct PhysicalEvent {
        PhysicalEventType _type;
        glm::vec2 _targetPosition;
    };

    enum SearchActionType {
        DIRECT,
        RAIL,
        NONE
    };

    enum SearchNodeType {
        SEARCH,
        SIMULATION
    };

    struct SearchNodeSearch {
        SearchActionType _action;
        std::string _ballId; // None if pocket?
        std::vector<PhysicalEvent> _events;
        std::unordered_map<std::string, Ball> _unusedBalls;
        billiard::search::Search _search;
        std::unordered_map<std::string, Ball> _state;
    };

    struct SearchNodeSimulation {
        node::System _simulation;
    };

    struct SearchNode {

        explicit SearchNode(SearchNodeType type,
                            std::variant<std::shared_ptr<SearchNodeSearch>, std::shared_ptr<SearchNodeSimulation>> body) :
                _parent(nullptr),
                _type(type),
                _body(std::move(body)),
                _cost(0),
                _isSolution(false) {
        }

        SearchNode(const SearchNode& data) = delete;
        SearchNode(SearchNode&& data) = delete;
        SearchNode operator=(const SearchNode& searchNode) = delete;
        SearchNode operator=(SearchNode&& searchNode) = delete;

        static std::shared_ptr<SearchNode> search(std::shared_ptr<SearchNode> parent = nullptr) {
            auto node = std::make_shared<SearchNode>(SearchNodeType::SEARCH, std::make_shared<SearchNodeSearch>());

            if (parent) {
                auto parentSearch = parent->asSearch();
                auto search = node->asSearch();

                search->_state = parentSearch->_state;
                search->_search = parentSearch->_search;
                search->_unusedBalls = parentSearch->_unusedBalls;
            }

            node->_parent = std::move(parent);

            return node;
        }

        static std::shared_ptr<SearchNode> simulation(std::shared_ptr<SearchNode> parent = nullptr) {
            auto node = std::make_shared<SearchNode>(SearchNodeType::SIMULATION, std::make_shared<SearchNodeSimulation>());

            node->_parent = std::move(parent);

            return node;
        }

        [[nodiscard]] std::vector<const SearchNode*> currentPath() const {
            std::vector<const SearchNode*> solution;
            auto node = this;
            while (node != nullptr || node->_type == SearchNodeType::SEARCH) {
                solution.push_back(this);
                node = node->_parent.get();
            }
            return solution;
        }

        [[nodiscard]] std::vector<const SearchNode*> path() const {
            std::vector<const SearchNode*> solution;
            auto node = this;
            while (node != nullptr) {
                solution.push_back(this);
                node = node->_parent.get();
            }
            return solution;
        }

        [[nodiscard]] std::vector<node::System> allSimulations() const {
            std::vector<node::System> allSystems;
            auto node = this;
            while (node != nullptr) {
                auto simulationNode = node->asSimulation();
                if (simulationNode) {
                    allSystems.push_back(simulationNode->_simulation);
                }
                node = node->_parent.get();
            }
            return allSystems;
        }

        std::shared_ptr<SearchNodeSearch> asSearch() {
            if (SearchNodeType::SEARCH == _type) {
                return std::get<std::shared_ptr<SearchNodeSearch>>(_body);
            }
            return nullptr;
        }

        [[nodiscard]] std::shared_ptr<SearchNodeSearch> asSearch() const {
            if (SearchNodeType::SEARCH == _type) {
                return std::get<std::shared_ptr<SearchNodeSearch>>(_body);
            }
            return nullptr;
        }

        std::shared_ptr<SearchNodeSimulation> asSimulation() {
            if (SearchNodeType::SIMULATION == _type) {
                return std::get<std::shared_ptr<SearchNodeSimulation>>(_body);
            }
            return nullptr;
        }

        [[nodiscard]] std::shared_ptr<SearchNodeSimulation> asSimulation() const {
            if (SearchNodeType::SIMULATION == _type) {
                return std::get<std::shared_ptr<SearchNodeSimulation>>(_body);
            }
            return nullptr;
        }

        std::shared_ptr<SearchNode> _parent;
        SearchNodeType _type;
        std::variant<std::shared_ptr<SearchNodeSearch>, std::shared_ptr<SearchNodeSimulation>> _body;

        // Interface
        uint64_t _cost;
        bool _isSolution;
    };

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
    prepareForSimulation(const std::shared_ptr<SearchNode>& input);
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
        auto unused = searchInput->_unusedBalls;
        for (auto& ball : unused) {

            if ((searchInput->_action != SearchActionType::NONE && ball.second._type == "WHITE") ||
                (ball.first == searchInput->_search._id || ball.second._type == searchInput->_search._type)) {

                auto result = expandSearchNodeByBall(input, state, searchInput, ball);
                expanded.insert(expanded.end(), result.begin(), result.end());
            }
        }

        return expanded;
    }

    std::vector<std::shared_ptr<SearchNode>>
    expandSearchNodeByBall(const std::shared_ptr<SearchNode>& input, const std::shared_ptr<SearchState>& state,
                         const std::shared_ptr<SearchNodeSearch>& searchInput,
                         const std::pair<std::string, Ball>& ball) {
        std::vector<std::shared_ptr<SearchNode>> expanded;

        auto result = SearchNode::search(input) ;// TODO: Remove and try to get result by expansion of ball.

        if (result) {
            auto resultSearchNode = result->asSearch();
            resultSearchNode->_unusedBalls.erase(resultSearchNode->_unusedBalls.find(ball.first));
            result->_cost = input->_cost + searchCost(resultSearchNode->_events);

            if (ball.second._type == "WHITE") {
                auto prepared = prepareForSimulation(result);
                expanded.insert(expanded.end(), prepared.begin(), prepared.end());
            } else {
                expanded.push_back(result);
            }
        }

        return expanded;
    }

    glm::vec2
    calculateMinimalForce(const std::vector<const SearchNode*>& nodes) {
        return glm::vec2{0, 0};
    }

    node::Layer toInputLayer(const std::shared_ptr<SearchNode>& input, const glm::vec2 force) {
        std::unordered_map<std::string, node::Node> nodes;

        auto asSearch = input->asSearch();

        for(auto& ball : asSearch->_state) {
            if (ball.second._type == "WHITE") {
                node::BallMovingNode initialEnergyNode{
                        state::BallState{
                                ball.second._position,
                                glm::vec2{0, 0}
                        },
                        state::BallState{
                                ball.second._position,
                                force}
                };
                node::Node result{node::NodeType::BALL_MOVING, ball.second._type, node::NodeVariant(initialEnergyNode)};
                nodes.insert({ball.first, result});
            } else {
                node::BallInRestNode inRestNode{
                        state::BallState{
                                ball.second._position,
                                glm::vec2{0, 0}
                        }
                };
                node::Node result{node::NodeType::BALL_IN_REST, ball.second._type, node::NodeVariant(inRestNode)};
                nodes.insert({ball.first, result});
            }
        }

        return node::Layer{0, nodes};
    }

    std::vector<std::shared_ptr<SearchNode>>
    prepareForSimulation(const std::shared_ptr<SearchNode>& input) {
        std::vector<std::shared_ptr<SearchNode>> expanded;

        auto minimalForce = calculateMinimalForce(input->currentPath());

        for (int i = 0; i < FORWARD_SEARCHES; ++i) {
            glm::vec2 increasedForce{
                minimalForce.x + (FORCE_STEP * i),
                minimalForce.y + (FORCE_STEP * i)
            };
            auto layer = toInputLayer(input, increasedForce);

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
                ball.second._type == searchInput->_search._type)) {
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
                auto prepared = prepareForSimulation(result);
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

    std::optional<event::Event> nextEvent(const node::Layer& layer, const std::shared_ptr<SearchState>& state);
    node::Layer createLayer(const event::Event& event, const node::Layer& previousLayer);
    std::string getTypeOf(const std::string& id, const std::string& type, const std::unordered_map<std::string, Ball>& state);
    uint64_t simulationCost(const node::System& system);

    std::vector<std::shared_ptr<SearchNode>>
    simulate(const std::shared_ptr<SearchNode>& input, const std::shared_ptr<SearchState>& state) {
        auto parentSearchInput = input->_parent->asSearch();
        auto simulationInput = input->asSimulation();
        auto& system = simulationInput->_simulation;
        std::vector<std::shared_ptr<SearchNode>> expanded;

        while(!system.final()) {
            auto event = nextEvent(system.lastLayer(), state);

            if (event) {
                auto layer = createLayer(*event, system.lastLayer());
                system.append(layer);

                if (layer.final()) {

                    if (!state->_config._rules._isValidEndState(
                            getTypeOf(parentSearchInput->_search._id,
                                      parentSearchInput->_search._type,
                                      parentSearchInput->_state),
                            layer)) {
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
                        auto typeToSearch = state->_config._rules._nextTypeToSearch(
                                parentSearchInput->_state.at(path.at(1)->asSearch()->_ballId)._type);
                        auto modifiedLayer = state->_config._rules._modifyState(layer);

                        std::vector<Ball> newBallPositions;
                        for (auto& node : modifiedLayer._nodes) {
                            if (node.second._type == node::NodeType::BALL_IN_REST) {
                                auto inRest = node.second.toInRest();
                                newBallPositions.emplace_back(Ball{
                                        inRest->_ball._position,
                                        parentSearchInput->_state.at(node.first)._type,
                                        node.first});
                            }
                        }

                        auto enrichedWithPockets = addPocketsAsInitialTargets(State{newBallPositions}, Search{"", typeToSearch}, state->_config);
                        for (auto& output : enrichedWithPockets) {
                            output->_parent = input;
                            output->_cost = input->_cost + cost;
                        }
                        expanded.insert(expanded.end(), enrichedWithPockets.begin(), enrichedWithPockets.end());
                    } else {
                        input->_isSolution = true;
                        input->_cost = cost;
                        expanded.emplace_back(input);
                    }
                }
            } else {
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
                      const std::unordered_map<std::string, node::Node>& balls);
    std::optional<event::Event>
    nextRailCollision(const std::pair<std::string, node::Node>& ball, const std::vector<Rail>& rails);
    std::optional<event::Event>
    nextPocketCollision(const std::pair<std::string, node::Node>& ball, const std::vector<Pocket>& pockets);
    std::optional<event::Event> min(const std::optional<event::Event>& event,
                                    const std::optional<event::Event>& currentEvent);

    std::optional<event::Event> nextEvent(const node::Layer& layer, const std::shared_ptr<SearchState>& state) {

        std::optional<event::Event> nextEvent = std::nullopt;

        for (auto& ball : layer.dynamicBalls()) {

            // läuft Kugel nächstens aus?
            nextEvent = min(nextBallInRest(ball), nextEvent);
            // Kollision mit statischer Kugelr
            nextEvent = min(nextBallCollision(ball, layer.staticBalls()), nextEvent);
            // Kollision mit anderer dynamischer Kugel
            nextEvent = min(nextBallCollision(ball, layer.dynamicBalls()), nextEvent);
            // Kollision mit Bande
            nextEvent = min(nextRailCollision(ball, state->_config._table._rails), nextEvent);
            // Kollision mit Zielloch
            nextEvent = min(nextPocketCollision(ball, state->_config._table._pockets), nextEvent);
        }

        return nextEvent ? std::make_optional<event::Event>(*nextEvent) : std::nullopt;
    }

    std::optional<event::Event> nextBallInRest(const std::pair<std::string, node::Node>& ball) {
        return std::nullopt; // TODO: Calculate Event
    }

    std::optional<event::Event>
    nextBallCollision(const std::pair<std::string, node::Node>& ball,
                      const std::unordered_map<std::string, node::Node>& balls) {
        return std::nullopt; // TODO: Calculate Event
    }

    std::optional<event::Event> nextRailCollision(const std::pair<std::string, node::Node>& ball, const std::vector<Rail>& rails) {
        return std::nullopt; // TODO: Calculate Event
    }

    std::optional<event::Event> nextPocketCollision(const std::pair<std::string, node::Node>& ball, const std::vector<Pocket>& pockets) {
        return std::nullopt; // TODO: Calculate Event
    }

    node::Layer createLayer(const event::Event& event, const node::Layer& previousLayer) {
        return node::Layer{}; // TODO: Create layer by event and previous layer
        // TODO: Calculate position of every other ball at this moment in time -> BallMovingNode or BallInRestNode
    }

    std::string getTypeOf(const std::string& id, const std::string& type, const std::unordered_map<std::string, Ball>& state) {
        if (!type.empty()) {
            return type;
        }

        return state.count(id) ? state.at(id)._type : "";
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
}

