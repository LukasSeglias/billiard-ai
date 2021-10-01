#pragma once

#include "macro_definition.hpp"

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <variant>
#include <unordered_map>
#include <functional>
#include <optional>

namespace billiard::search {

    struct EXPORT_BILLIARD_SEARCH_LIB Search {
        Search(std::string id = "", std::string type = "");

        std::string _id;
        std::string _type;
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

    struct EXPORT_BILLIARD_SEARCH_LIB Rail{
            Rail(std::string id, const glm::vec2& start, const glm::vec2& end);

            std::string _id;
            glm::vec2 _start;
            glm::vec2 _end;
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

        struct EXPORT_BILLIARD_SEARCH_LIB Event {
            Event(EventType type, float time,
                  std::variant<BallCollision, BallRailCollision, BallPotting, BallInRest> body);

            EventType _type;
            float _time;
            std::variant<BallCollision, BallRailCollision, BallPotting, BallInRest> _body;
        };
    }

    namespace state {

        struct EXPORT_BILLIARD_SEARCH_LIB BallState {
            BallState(const glm::vec2& position, const glm::vec2& velocity);

            glm::vec2 _position;
            glm::vec2 _velocity;
        };

        struct EXPORT_BILLIARD_SEARCH_LIB PocketState {
            PocketState(std::string id, const glm::vec2& position);

            std::string _id;
            glm::vec2 _position;
        };
    }

    namespace node {

        enum NodeType {
            BALL_MOVING,
            BALL_COLLISION,
            BALL_RAIL_COLLISION,
            BALL_POTTING,
            BALL_IN_REST, // TODO: besserer name
            POCKET
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
            BallPottingNode(const state::BallState& before, const state::BallState& after,
                            event::BallPotting cause);
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

        struct EXPORT_BILLIARD_SEARCH_LIB PocketNode {
            explicit PocketNode(const state::PocketState& pocket);
            PocketNode(const PocketNode& node) = default;

            state::PocketState _pocket;
        };

        using NodeVariant = std::variant<BallCollisionNode, BallRailCollisionNode, BallPottingNode, BallInRestNode, BallShotNode, BallNode, BallMovingNode, PocketNode>;

        struct EXPORT_BILLIARD_SEARCH_LIB Node {
            Node(NodeType type,NodeVariant body);
            Node& operator=(const Node& node) = default;

            std::optional<BallInRestNode> toInRest();
            std::optional<BallPottingNode> toPotted();

            NodeType _type;
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

        private:
            static std::unordered_map<std::string, Node> getDynamic(const std::unordered_map<std::string, Node>& nodes);
            static std::unordered_map<std::string, Node> getStatic(const std::unordered_map<std::string, Node>& nodes);
        };

        struct EXPORT_BILLIARD_SEARCH_LIB System {
            System();

            [[nodiscard]] const Layer& lastLayer() const;
            [[nodiscard]] bool empty() const;
            [[nodiscard]] bool final() const;

            std::vector<Layer> _layers;
        };
    }

    struct EXPORT_BILLIARD_SEARCH_LIB Configuration {

        struct {
            float _radius = 0; // in millimeters
        } _ball;

        struct {
            glm::vec2 _center = glm::vec2{0, 0};
            // Pockets
            // o--o--o
            // |     |
            // o--o--o
            std::vector<Pocket> _pockets;
            std::vector<Rail> _rails;
            // TODO: deceleration?
        } _table;

        struct {
            std::function<std::string (const std::string&)> _nextTypeToSearch = [](const std::string& previousType) {return "";};
            std::function<node::Layer (const node::Layer&)> _modifyState = [](const node::Layer& layer) {return layer;};
            std::function<bool(const std::string&, const node::Layer&)> _isValidEndState = [](
                    const std::string& expectedPottedType, const node::Layer& layer) { return true; };
        } _rules;
    };
}