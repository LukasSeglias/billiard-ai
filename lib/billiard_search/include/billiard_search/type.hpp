#pragma once

#include "macro_definition.hpp"

#include <vector>
#include <set>
#include <string>
#include <glm/glm.hpp>
#include <variant>
#include <unordered_map>
#include <functional>
#include <optional>
#include <atomic>
#include <algorithm>

namespace billiard::search {

    struct EXPORT_BILLIARD_SEARCH_LIB Search {
        Search(std::string id = "", std::vector<std::string> types = {});

        std::string _id;
        std::vector<std::string> _types;
    };

    struct EXPORT_BILLIARD_SEARCH_LIB Ball {
        Ball(const glm::vec2& position, std::string type, std::string id);

        bool operator==(const Ball& other) const;

        glm::vec2 _position;
        std::string _type;
        std::string _id;
    };

    struct EXPORT_BILLIARD_SEARCH_LIB State {
        explicit State(std::vector<Ball> balls);

        std::vector<Ball> _balls;
    };

    enum PocketType {
        CORNER,
        CENTER
    };

    struct EXPORT_BILLIARD_SEARCH_LIB PocketPottingPoint { // TODO: LAST: use or remove
        PocketPottingPoint(glm::vec2 position);

        glm::vec2 _position;
    };

    struct EXPORT_BILLIARD_SEARCH_LIB Pocket {
        Pocket(std::string id, PocketType type, glm::vec2 position, glm::vec2 normal, float radius);

        std::string _id;
        PocketType _type;
        glm::vec2 _position;
        glm::vec2 _normal;
        float _radius;
        //std::vector<PocketPottingPoint> _pottingPoints; // TODO: LAST: use or remove
    };

    struct EXPORT_BILLIARD_SEARCH_LIB Rail {
        Rail(std::string id, const glm::vec2& start, const glm::vec2& end, const glm::vec2& shiftDirection, float ballRadius);

        std::string _id;
        glm::vec2 _start;
        glm::vec2 _end;
        glm::vec2 _normal;
        glm::vec2 _shiftedStart; // Bandenstart um den Radius der Kugel zum Zentrum verschoben
        glm::vec2 _shiftedEnd; // Bandenende um den Radius der Kugel zum Zentrum verschoben
        glm::mat3 _reflectionMatrix;

    private:
        glm::vec2 shift(glm::vec2 position, float ballRadius, const glm::vec2& normal);
        glm::mat3x3 calculateReflectionMatrix(const glm::vec2& start, const glm::vec2& normal);
    };

    namespace event {

        enum EventType {
            // Two balls collide
            BALL_COLLISION,
            // Ball collides with rail
            BALL_RAIL_COLLISION,
            // Ball rolls into pocket
            BALL_POTTING,
            // Ball stops moving
            BALL_IN_REST,
            // Ball is rolling
            BALL_ROLLING
        };

        struct EXPORT_BILLIARD_SEARCH_LIB BallCollision {
            BallCollision(std::string ball1, std::string ball2);

            std::string _ball1;
            std::string _ball2;
        };

        struct EXPORT_BILLIARD_SEARCH_LIB BallRailCollision {
            BallRailCollision(std::string ball, std::string rail);

            std::string _ball;
            std::string _rail;
        };

        struct EXPORT_BILLIARD_SEARCH_LIB BallPotting {
            BallPotting(std::string ball, std::string pocket);

            std::string _ball;
            std::string _pocket;
        };

        struct EXPORT_BILLIARD_SEARCH_LIB BallInRest {
            explicit BallInRest(std::string ball);

            std::string _ball;
        };

        struct EXPORT_BILLIARD_SEARCH_LIB BallRolling {
            explicit BallRolling(std::string ball);

            std::string _ball;
        };

        using EventVariant = std::variant<BallCollision, BallRailCollision, BallPotting, BallInRest, BallRolling>;

        struct EXPORT_BILLIARD_SEARCH_LIB Event {
            Event(EventType type, float time, EventVariant body);

            EventType _type;
            float _time;
            EventVariant _body;

            [[nodiscard]] std::optional<BallPotting> toPotting() const;
            [[nodiscard]] std::optional<BallRailCollision> toRailCollision() const;
            [[nodiscard]] std::optional<BallCollision> toBallCollision() const;
            [[nodiscard]] std::optional<BallInRest> toInRest() const;
            [[nodiscard]] std::optional<BallRolling> toRolling() const;
            [[nodiscard]] std::set<std::string> affected() const;
        };
    }

    namespace state {

        struct EXPORT_BILLIARD_SEARCH_LIB BallState {
            BallState(const glm::vec2& position, const glm::vec2& velocity, float accelerationLength,
                      float slideAccelerationLength, bool isRolling = false);

            glm::vec2 _position;
            glm::vec2 _velocity;
            glm::vec2 _velocityNormed;
            glm::vec2 _acceleration;
            bool _isRolling;
        };
    }

    namespace node {

        enum NodeType {
            // Ball is rolling
            BALL_MOVING,
            // Two balls collide
            BALL_COLLISION,
            // Ball collides with rail
            BALL_RAIL_COLLISION,
            // Ball rolls into pocket
            BALL_POTTING,
            // Ball is stroke with the cue stick
            BALL_SHOT,
            // Ball stops moving
            BALL_IN_REST
        };

        struct EXPORT_BILLIARD_SEARCH_LIB BallMovingNode {
            BallMovingNode(const state::BallState& before, const state::BallState& after);
            BallMovingNode(const BallMovingNode& node) = default;

            state::BallState _before;
            state::BallState _after;
        };

        struct EXPORT_BILLIARD_SEARCH_LIB BallCollisionNode {
            BallCollisionNode(const state::BallState& before, const state::BallState& after,
                              event::BallCollision cause);
            BallCollisionNode(const BallCollisionNode& node) = default;

            state::BallState _before;
            state::BallState _after;
            event::BallCollision _cause;
        };

        struct EXPORT_BILLIARD_SEARCH_LIB BallRailCollisionNode {
            BallRailCollisionNode(const state::BallState& before, const state::BallState& after,
                                  event::BallRailCollision cause);
            BallRailCollisionNode(const BallRailCollisionNode& node) = default;

            state::BallState _before;
            state::BallState _after;
            event::BallRailCollision _cause;
        };

        struct EXPORT_BILLIARD_SEARCH_LIB BallPottingNode {
            BallPottingNode(const state::BallState& before, const state::BallState& after, event::BallPotting cause);
            BallPottingNode(const BallPottingNode& node) = default;

            state::BallState _before;
            state::BallState _after;
            event::BallPotting _cause;
        };

        struct EXPORT_BILLIARD_SEARCH_LIB BallInRestNode {
            explicit BallInRestNode(const state::BallState& ball);
            BallInRestNode(const BallInRestNode& node) = default;

            state::BallState _ball;
        };

        struct EXPORT_BILLIARD_SEARCH_LIB BallShotNode {
            explicit BallShotNode(const state::BallState& ball);
            BallShotNode(const BallShotNode& node) = default;

            state::BallState _ball;
        };

        struct EXPORT_BILLIARD_SEARCH_LIB BallNode {
            explicit BallNode(const state::BallState& ball);
            BallNode(const BallNode& node) = default;

            state::BallState _ball;
        };

        using NodeVariant = std::variant<BallCollisionNode, BallRailCollisionNode, BallPottingNode, BallInRestNode, BallShotNode, BallNode, BallMovingNode>;

        struct EXPORT_BILLIARD_SEARCH_LIB Node {
            Node(NodeType type, std::string ballType, NodeVariant body);
            Node& operator=(const Node& node) = default;

            [[nodiscard]] std::optional<BallInRestNode> toInRest() const;
            [[nodiscard]] std::optional<BallPottingNode> toPotted() const;
            [[nodiscard]] std::optional<BallCollisionNode> toBallCollision() const;
            [[nodiscard]] std::optional<BallRailCollisionNode> toBallRailCollision() const;
            [[nodiscard]] std::optional<BallShotNode> toBallShot() const;
            [[nodiscard]] std::optional<BallMovingNode> toBallMoving() const;

            [[nodiscard]] std::optional<state::BallState> before() const;
            [[nodiscard]] std::optional<state::BallState> after() const;
            [[nodiscard]] std::string getInvolvedId() const;

            [[nodiscard]] bool isStatic() const;


            NodeType _type;
            std::string _ballType;
            NodeVariant _body;
        };

        struct EXPORT_BILLIARD_SEARCH_LIB Layer {
            explicit Layer(float time = 0,
                           std::unordered_map<std::string, Node> nodes = std::unordered_map<std::string, Node>{});

            [[nodiscard]] bool final() const;
            [[nodiscard]] std::unordered_map<std::string, Node> dynamicBalls() const;
            [[nodiscard]] std::unordered_map<std::string, Node> staticBalls() const;

            float _time;
            std::unordered_map<std::string, Node> _dynamic;
            std::unordered_map<std::string, Node> _static;
            std::unordered_map<std::string, Node> _nodes;
            bool _isFirst;
            bool _isLast;

        private:
            static std::unordered_map<std::string, Node> getDynamic(const std::unordered_map<std::string, Node>& nodes);
            static std::unordered_map<std::string, Node> getStatic(const std::unordered_map<std::string, Node>& nodes);
        };

        struct EXPORT_BILLIARD_SEARCH_LIB System {
            System();

            [[nodiscard]] const Layer& firstLayer() const;
            [[nodiscard]] const Layer& lastLayer() const;
            [[nodiscard]] bool empty() const;
            [[nodiscard]] bool final() const;
            void append(Layer layer);

            std::vector<Layer> _layers;
            std::unordered_map<std::string, std::string> _lastRailCollisions;
            std::unordered_map<std::string, float> _nextRolling;
            unsigned int _id;
        };
    }

    struct EXPORT_BILLIARD_SEARCH_LIB Configuration {

        struct {
            float _radius = 0; // in millimeters
            float _diameterSquared = 0;
            float _diameter = 0;
        } _ball;

        struct {
            // Pockets
            // o--o--o
            // |     |
            // o--o--o
            std::vector<Pocket> _pockets;
            std::unordered_map<std::string, Rail> _rails;
            // Minimale erwünschte Geschwindigkeit einer Kugel zum Zeitpunkt, wenn diese ins Loch rollt.
            float minimalPocketVelocity;
            // Quadrierte diagonale Länge des Spielfelds
            float diagonalLengthSquared;
        } _table;

        struct {
            std::function<Search (const State&, const std::vector<std::string>&)> _nextSearch =
                    [](const State& state, const std::vector<std::string>& previousTypes) {return Search{};};
            std::function<State (const State&, const std::unordered_map<std::string, std::string>&)> _modifyState =
                    [](const State& state, const std::unordered_map<std::string, std::string>& ids) {return state;};
            std::function<bool(const std::vector<std::string>&, const node::Layer&)> _isValidEndState = [](
                    const std::vector<std::string>& expectedTypes, const node::Layer& layer) { return true; };
            std::function<double(const std::string&)> _scoreForPottedBall = [](const std::string& ballType) { return 0.0; };
        } _rules;

        bool _depthSearchEnabled = true;
    };

    struct EXPORT_BILLIARD_SEARCH_LIB SearchState {

        Configuration _config;

        struct {
            float _accelerationLength = 0.0;
            float _slideAccelerationLength = 0.0;
        } _ball;
    };

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
            },
            {
                type: BALL_KICK,
                targetPosition: red's position
            }]
    };
    lightred = {
            parent: pocket,
            action: DIRECT,
            ballId: red
            events: [{
                type: BALL_COLLISION,
                targetPosition: 1
            },
            {
                type: BALL_KICK,
                targetPosition: lightred's position
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
        /**
         * ID
         * - of other ball in case of BALL_COLLISION
         * - of rail in case of RAIL_COLLISION
         * - of pocket in case of POCKET_COLLISION
         * - is empty in case of BALL_KICK
         */
        std::string id;
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
        std::string _ballId; // => Pocket-ID if action = SearchActionType::NONE
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
                _searchCost(0),
                _searchDepth(0),
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

                if (parent->_type == SearchNodeType::SEARCH) {
                    node->_searchDepth = parent->_searchDepth + 1;
                } else {
                    node->_searchDepth = 0;
                }
            }

            node->_parent = std::move(parent);

            return node;
        }

        static std::shared_ptr<SearchNode> simulation(std::shared_ptr<SearchNode> parent = nullptr) {
            auto node = std::make_shared<SearchNode>(SearchNodeType::SIMULATION, std::make_shared<SearchNodeSimulation>());

            static std::atomic<unsigned int> id { 1 };

            node->asSimulation()->_simulation._id = id++;
            if (parent) {
                node->_cost = parent->_cost;
                node->_searchCost = parent->_searchCost;
                node->_searchDepth = 0;
                node->_parent = std::move(parent);
            }

            return node;
        }

        [[nodiscard]] std::vector<const SearchNode*> currentPath() const {
            std::vector<const SearchNode*> solution;
            auto node = this;
            while (node != nullptr && node->_type == SearchNodeType::SEARCH) {
                solution.push_back(node);
                node = node->_parent.get();
            }
            return solution;
        }

        [[nodiscard]] std::vector<const SearchNode*> path() const {
            std::vector<const SearchNode*> solution;
            auto node = this;
            while (node != nullptr) {
                solution.push_back(node);
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
            std::reverse(allSystems.begin(), allSystems.end());
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
        uint64_t _searchCost;
        uint16_t _searchDepth;
        bool _isSolution;
    };
}