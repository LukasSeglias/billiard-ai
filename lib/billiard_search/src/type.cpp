#include <billiard_search/type.hpp>
#include <algorithm>

billiard::search::Search::Search(std::string id, std::string type) :
    _id(std::move(id)),
    _type(std::move(type)) {
}

billiard::search::Ball::Ball(const glm::vec2& position, std::string type, std::string id) :
    _position(position),
    _type(std::move(type)),
    _id(std::move(id)) {
}

billiard::search::State::State(std::vector<Ball> balls) :
    _balls(std::move(balls)) {
}

billiard::search::PocketPottingPoint::PocketPottingPoint(glm::vec2 position) :
    _position(position) {
}

billiard::search::Pocket::Pocket(std::string id, PocketType type, glm::vec2 position, float radius) :
        _id(std::move(id)),
        _type(type),
        _position(position),
        _radius(radius) {
}

billiard::search::Rail::Rail(std::string id, const glm::vec2& start, const glm::vec2& end) :
        _id(std::move(id)),
        _start(start),
        _end(end) {
}

billiard::search::event::BallCollision::BallCollision(std::string ball1, std::string ball2) :
    _ball1(std::move(ball1)),
    _ball2(std::move(ball2)) {
}

billiard::search::event:: BallRailCollision::BallRailCollision(std::string ball, std::string rail) :
    _ball(std::move(ball)),
    _rail(std::move(rail)) {
}

billiard::search::event::BallPotting::BallPotting(std::string ball, std::string pocket) :
    _ball(std::move(ball)),
    _pocket(std::move(pocket)) {
}

billiard::search::event::BallInRest::BallInRest(std::string ball) :
    _ball(std::move(ball)) {
}

billiard::search::event::Event::Event(EventType type, float time,
                               std::variant<BallCollision, BallRailCollision, BallPotting, BallInRest> body) :
                               _type(type),
                               _time(time),
                               _body(std::move(body)) {
}

billiard::search::state::BallState::BallState(const glm::vec2& position, const glm::vec2& velocity) :
    _position(position),
    _velocity(velocity) {
}

billiard::search::state::PocketState::PocketState(std::string id, const glm::vec2& position) :
    _id(id),
    _position(position) {
}

billiard::search::node::BallMovingNode::BallMovingNode(const state::BallState& before, const state::BallState& after) :
        _before(before),
        _after(after) {
}

billiard::search::node::BallCollisionNode::BallCollisionNode(const state::BallState& before,
                                                             const state::BallState& after,
                                                             event::BallCollision cause) :
                                          _before(before),
                                          _after(after),
                                          _cause(std::move(cause)) {
}

billiard::search::node::BallRailCollisionNode::BallRailCollisionNode(const state::BallState& before,
                                                                     const state::BallState& after,
                                              event::BallRailCollision cause) :
                                              _before(before),
                                              _after(after),
                                              _cause(std::move(cause)) {
}

billiard::search::node::BallPottingNode::BallPottingNode(const state::BallState& before, const state::BallState& after,
                                        event::BallPotting cause) :
                                        _before(before),
                                        _after(after),
                                        _cause(std::move(cause)) {
}

billiard::search::node::BallInRestNode::BallInRestNode(const state::BallState& ball) :
    _ball(ball) {
}

billiard::search::node::BallShotNode::BallShotNode(const state::BallState& ball) :
    _ball(ball) {
}

billiard::search::node::BallNode::BallNode(const state::BallState& ball) :
    _ball(ball) {
}

billiard::search::node::PocketNode::PocketNode(const state::PocketState& pocket) :
    _pocket(pocket) {
}

billiard::search::node::Node::Node(NodeType type, NodeVariant body)
        :
        _type(type),
        _body(std::move(body)) {
}

std::optional<billiard::search::node::BallInRestNode> billiard::search::node::Node::toInRest() {
    if (_type == NodeType::BALL_IN_REST) {
        return std::make_optional(std::get<BallInRestNode>(_body));
    }
    return std::nullopt;
}

std::optional<billiard::search::node::BallPottingNode> billiard::search::node::Node::toPotted() {
    if (_type == NodeType::BALL_POTTING) {
        return std::make_optional(std::get<BallPottingNode>(_body));
    }
    return std::nullopt;
}

billiard::search::node::Layer::Layer(float time, std::unordered_map<std::string, Node> nodes) :
    _time(time),
    _dynamic(getDynamic(nodes)),
    _static(getStatic(nodes)),
    _nodes(std::move(nodes)) {
}

bool billiard::search::node::Layer::final() const {
    return std::all_of(_nodes.begin(), _nodes.end(), [](const std::pair<std::string, node::Node>& node) {
        return node.second._type == NodeType::BALL_IN_REST || node.second._type == NodeType::BALL_POTTING;
    });
}

std::unordered_map<std::string, billiard::search::node::Node> billiard::search::node::Layer::dynamicBalls() const {
    return _dynamic;
}

std::unordered_map<std::string, billiard::search::node::Node> billiard::search::node::Layer::staticBalls() const {
    return _static;
}

std::unordered_map<std::string, billiard::search::node::Node>
billiard::search::node::Layer::getDynamic(const std::unordered_map<std::string, Node>& nodes) {
    static const std::vector<billiard::search::node::NodeType> dynamicTypes{
            billiard::search::node::NodeType::BALL_MOVING,
            billiard::search::node::NodeType::BALL_COLLISION,
            billiard::search::node::NodeType::BALL_RAIL_COLLISION
    };

    std::unordered_map<std::string, billiard::search::node::Node> result;

    for(auto& node : nodes) {
        for (auto& dynamicType : dynamicTypes) {
            if (node.second._type == dynamicType) {
                result.insert(node);
                break;
            }
        }
    }

    return result;
}

std::unordered_map<std::string, billiard::search::node::Node>
billiard::search::node::Layer::getStatic(const std::unordered_map<std::string, Node>& nodes) {
    static const std::vector<billiard::search::node::NodeType> staticTypes{
            billiard::search::node::NodeType::BALL_POTTING,
            billiard::search::node::NodeType::BALL_IN_REST
    };

    std::unordered_map<std::string, billiard::search::node::Node> result;

    for(auto& node : nodes) {
        for (auto& staticType : staticTypes) {
            if (node.second._type == staticType) {
                result.insert(node);
                break;
            }
        }
    }

    return result;
}

billiard::search::node::System::System() :
    _layers() {
}

const billiard::search::node::Layer& billiard::search::node::System::lastLayer() const {
    return _layers.at(_layers.size() - 1);
}

bool billiard::search::node::System::empty() const {
    return _layers.empty();
}

bool billiard::search::node::System::final() const {
    return !empty() && lastLayer().final();
}
