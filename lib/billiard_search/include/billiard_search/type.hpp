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

    struct EXPORT_BILLIARD_SEARCH_LIB PocketPottingPoint {
        PocketPottingPoint(glm::vec2 position);

        glm::vec2 _position;
        // TODO:
    };

    struct EXPORT_BILLIARD_SEARCH_LIB Pocket {
        Pocket(std::string id, PocketType type, glm::vec2 position, float radius);

        std::string _id;
        PocketType _type;
        glm::vec2 _position;
        float _radius;
        //std::vector<PocketPottingPoint> _pottingPoints;
    };

    enum RailLocation {
        TOP,
        BOTTOM,
        RIGHT,
        LEFT
    };

    struct EXPORT_BILLIARD_SEARCH_LIB Rail {
        Rail(std::string id, const glm::vec2& start, const glm::vec2& end, float ballRadius, const RailLocation& location);

        std::string _id;
        glm::vec2 _start;
        glm::vec2 _end;
        RailLocation _location;
        glm::vec2 _shiftedStart; // Bandenstart um den Radius der Kugel zum Zentrum verschoben
        glm::vec2 _shiftedEnd; // Bandenende um den Radius der Kugel zum Zentrum verschoben
        glm::vec2 _normal;

    private:
        glm::vec2 shift(glm::vec2 position, float ballRadius, const RailLocation& railType);
    };

    namespace event {

        enum EventType {
            BALL_COLLISION,
            BALL_RAIL_COLLISION,
            BALL_POTTING,
            BALL_IN_REST // TODO: besserer name
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

        struct EXPORT_BILLIARD_SEARCH_LIB BallInRest { // TODO: besserer name
            explicit BallInRest(std::string ball);

            std::string _ball;
        };

        using EventVariant = std::variant<BallCollision, BallRailCollision, BallPotting, BallInRest>;

        struct EXPORT_BILLIARD_SEARCH_LIB Event {
            Event(EventType type, float time, EventVariant body);

            EventType _type;
            float _time;
            EventVariant _body;

            [[nodiscard]] std::optional<BallPotting> toPotting() const;
            [[nodiscard]] std::optional<BallRailCollision> toRailCollision() const;
            [[nodiscard]] std::optional<BallCollision> toBallCollision() const;
            [[nodiscard]] std::optional<BallInRest> toInRest() const;
            [[nodiscard]] std::set<std::string> affected() const;
        };
    }

    namespace state {

        struct EXPORT_BILLIARD_SEARCH_LIB BallState {
            BallState(const glm::vec2& position, const glm::vec2& velocity, float accelerationLength);

            glm::vec2 _position;
            glm::vec2 _velocity;
            glm::vec2 _velocityNormed;
            glm::vec2 _acceleration;
        };
    }

    namespace node {

        enum NodeType {
            BALL_MOVING,
            BALL_COLLISION,
            BALL_RAIL_COLLISION,
            BALL_POTTING,
            BALL_SHOT,
            BALL_IN_REST // TODO: besserer name
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

        struct EXPORT_BILLIARD_SEARCH_LIB BallInRestNode { // TODO: besserer name
            explicit BallInRestNode(const state::BallState& ball);
            BallInRestNode(const BallInRestNode& node) = default;

            state::BallState _ball;
        };

        struct EXPORT_BILLIARD_SEARCH_LIB BallShotNode { // TODO: besserer name
            explicit BallShotNode(const state::BallState& ball);
            BallShotNode(const BallShotNode& node) = default;

            state::BallState _ball;
        };

        struct EXPORT_BILLIARD_SEARCH_LIB BallNode { // TODO: besserer name
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

            [[nodiscard]] bool isStatic() const;


            NodeType _type;
            std::string _ballType;
            NodeVariant _body; // TODO: besserer name
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

            [[nodiscard]] const Layer& lastLayer() const;
            [[nodiscard]] bool empty() const;
            [[nodiscard]] bool final() const;
            void append(Layer layer);

            std::vector<Layer> _layers;
            std::unordered_map<std::string, std::string> _lastRailCollisions;
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
            glm::vec2 _center = glm::vec2{0, 0};
            // Pockets
            // o--o--o
            // |     |
            // o--o--o
            std::vector<Pocket> _pockets;
            std::vector<Rail> _rails;
            // Minimale erwünschte Geschwindigkeit einer Kugel zum Zeitpunkt, wenn diese ins Loch rollt.
            float minimalPocketVelocity;
            // TODO: deceleration?
        } _table;

        struct {
            std::function<Search (const Search&, const std::vector<std::string>&)> _nextSearch =
                    [](const Search& previousType, const std::vector<std::string>& previousTypes) {return Search{};};
            std::function<node::Layer (const node::Layer&)> _modifyState = [](const node::Layer& layer) {return layer;};
            std::function<bool(const std::vector<std::string>&, const node::Layer&)> _isValidEndState = [](
                    const std::vector<std::string>& expectedTypes, const node::Layer& layer) { return true; };
        } _rules;
    };

    struct EXPORT_BILLIARD_SEARCH_LIB SearchState {

        Configuration _config;

        struct {
            float _accelerationLength = 0.0;
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

            static std::atomic<unsigned int> id { 1 };

            node->asSimulation()->_simulation._id = id++;
            node->_parent = std::move(parent);

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
        bool _isSolution;
    };
}