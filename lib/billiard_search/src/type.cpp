#include <billiard_search/type.hpp>
#include <algorithm>
#include <billiard_physics/billiard_physics.hpp>

billiard::search::Search::Search(std::string id, std::vector<std::string> types) :
    _id(std::move(id)),
    _types(std::move(types)) {
}

billiard::search::Ball::Ball(const glm::vec2& position, std::string type, std::string id) :
    _position(position),
    _type(std::move(type)),
    _id(std::move(id)) {
}

bool billiard::search::Ball::operator==(const Ball& other) const {
    return _position == other._position &&
           _type == other._type &&
           _id == other._id;
}

billiard::search::State::State(std::vector<Ball> balls) :
    _balls(std::move(balls)) {
}

billiard::search::Pocket::Pocket(std::string id, PocketType type, glm::vec2 position, std::vector<glm::vec2> pottingPoints, glm::vec2 normal, float radius) :
        _id(std::move(id)),
        _type(type),
        _position(position),
        _pottingPoints(pottingPoints),
        _normal(normal),
        _radius(radius) {
}

billiard::search::SearchPocket::SearchPocket(std::string id, PocketType type, glm::vec2 pottingPoint, glm::vec2 normal, float radius) :
        _id(std::move(id)),
        _type(type),
        _pottingPoint(pottingPoint),
        _normal(normal),
        _radius(radius) {
}

std::vector<billiard::search::SearchPocket> billiard::search::buildSearchPocketsOfPockets(const std::vector<Pocket>& pockets) {
    std::vector<SearchPocket> result;

    for (auto& pocket : pockets) {
        int number = 1;
        for (auto& pottingPoint : pocket._pottingPoints) {
            result.emplace_back(pocket._id + std::to_string(number++), pocket._type, pottingPoint, pocket._normal, pocket._radius);
        }
    }

    return result;
}

billiard::search::Rail::Rail(std::string id,
                             const glm::vec2& start,
                             const glm::vec2& end,
                             const glm::vec2& shiftDirection,
                             float ballRadius) :
        _id(std::move(id)),
        _start(start),
        _end(end),
        _normal(glm::normalize(billiard::physics::perp(end - start))),
        _shiftedStart(shift(start, ballRadius, shiftDirection)),
        _shiftedEnd(shift(end, ballRadius, shiftDirection)),
        _reflectionMatrix(calculateReflectionMatrix(_shiftedStart, _normal)) {
}

glm::vec2 billiard::search::Rail::shift(glm::vec2 position, float ballRadius, const glm::vec2& shiftDirection) {
    return position + shiftDirection * ballRadius;
}

glm::mat3x3 billiard::search::Rail::calculateReflectionMatrix(const glm::vec2& start, const glm::vec2& normal) {
    auto s = glm::abs(normal) * -2.0f + glm::vec2{1.0f, 1.0f};
    return glm::mat3x3{
            s.x, 0, 0,
            0, s.y, 0,
            start.x - s.x * start.x,start.y - s.y * start.y,1
    };
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

billiard::search::event::BallRolling::BallRolling(std::string ball) :
        _ball(std::move(ball)) {
}

billiard::search::event::Event::Event(EventType type, float time, EventVariant body) :
                               _type(type),
                               _time(time),
                               _body(std::move(body)) {
}

std::optional<billiard::search::event::BallPotting> billiard::search::event::Event::toPotting() const {
    if (_type == EventType::BALL_POTTING) {
        return std::get<BallPotting>(_body);
    }
    return std::nullopt;
}

std::optional<billiard::search::event::BallRailCollision> billiard::search::event::Event::toRailCollision() const {
    if (_type == EventType::BALL_RAIL_COLLISION) {
        return std::get<BallRailCollision>(_body);
    }
    return std::nullopt;
}

std::optional<billiard::search::event::BallCollision> billiard::search::event::Event::toBallCollision() const {
    if (_type == EventType::BALL_COLLISION) {
        return std::get<BallCollision>(_body);
    }
    return std::nullopt;
}

std::optional<billiard::search::event::BallInRest> billiard::search::event::Event::toInRest() const {
    if (_type == EventType::BALL_IN_REST) {
        return std::get<BallInRest>(_body);
    }
    return std::nullopt;
}

std::optional<billiard::search::event::BallRolling> billiard::search::event::Event::toRolling() const {
    if (_type == EventType::BALL_ROLLING) {
        return std::get<BallRolling>(_body);
    }
    return std::nullopt;
}

std::set<std::string> billiard::search::event::Event::affected() const {
    auto asPotting = toPotting();
    if (asPotting) {
        return std::set<std::string>{asPotting->_ball};
    }

    auto asBallCollision = toBallCollision();
    if (asBallCollision) {
        return std::set<std::string>{asBallCollision->_ball1, asBallCollision->_ball2};
    }

    auto asRailCollision = toRailCollision();
    if (asRailCollision) {
        return std::set<std::string>{asRailCollision->_ball};
    }

    auto asInRest = toInRest();
    if (asInRest) {
        return std::set<std::string>{asInRest->_ball};
    }

    auto asInRolling = toRolling();
    if (asInRolling) {
        return std::set<std::string>{asInRolling->_ball};
    }

    return std::set<std::string>{};
}

billiard::search::state::BallState::BallState(const glm::vec2& position, const glm::vec2& velocity,
                                              float accelerationLength, float slideAccelerationLength, bool isRolling) :
    _position(position),
    _velocity(velocity),
    _velocityNormed(billiard::physics::normalize(velocity)),
    _acceleration(billiard::physics::accelerationByNormed(_velocityNormed,
                                                          isRolling ? accelerationLength : slideAccelerationLength)),
    _isRolling(isRolling) {
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

billiard::search::node::Node::Node(NodeType type, std::string ballType, NodeVariant body)
        :
        _type(type),
        _ballType(std::move(ballType)),
        _body(std::move(body)) {
}

std::optional<billiard::search::node::BallInRestNode> billiard::search::node::Node::toInRest() const {
    if (_type == NodeType::BALL_IN_REST) {
        return std::make_optional(std::get<BallInRestNode>(_body));
    }
    return std::nullopt;
}

std::optional<billiard::search::node::BallPottingNode> billiard::search::node::Node::toPotted() const {
    if (_type == NodeType::BALL_POTTING) {
        return std::make_optional(std::get<BallPottingNode>(_body));
    }
    return std::nullopt;
}

std::optional<billiard::search::node::BallCollisionNode> billiard::search::node::Node::toBallCollision() const {
    if (_type == NodeType::BALL_COLLISION) {
        return std::make_optional(std::get<BallCollisionNode>(_body));
    }
    return std::nullopt;
}

std::optional<billiard::search::node::BallRailCollisionNode> billiard::search::node::Node::toBallRailCollision() const {
    if (_type == NodeType::BALL_RAIL_COLLISION) {
        return std::make_optional(std::get<BallRailCollisionNode>(_body));
    }
    return std::nullopt;
}

std::optional<billiard::search::node::BallShotNode> billiard::search::node::Node::toBallShot() const {
    if (_type == NodeType::BALL_SHOT) {
        return std::make_optional(std::get<BallShotNode>(_body));
    }
    return std::nullopt;
}

std::optional<billiard::search::node::BallMovingNode> billiard::search::node::Node::toBallMoving() const {
    if (_type == NodeType::BALL_MOVING) {
        return std::make_optional(std::get<BallMovingNode>(_body));
    }
    return std::nullopt;
}

std::optional<billiard::search::state::BallState> billiard::search::node::Node::before() const {
    auto inRest = toInRest();
    if (inRest) {
        return inRest->_ball;
    }

    auto potted = toPotted();
    if(potted) {
        return potted->_before;
    }

    auto ballCollision = toBallCollision();
    if(ballCollision) {
        return ballCollision->_before;
    }

    auto ballRailCollision = toBallRailCollision();
    if(ballRailCollision) {
        return ballRailCollision->_before;
    }

    auto ballMoving = toBallMoving();
    if(ballMoving) {
        return ballMoving->_before;
    }

    return std::nullopt;
}

std::optional<billiard::search::state::BallState> billiard::search::node::Node::after() const {
    auto inRest = toInRest();
    if (inRest) {
        return inRest->_ball;
    }

    auto potted = toPotted();
    if(potted) {
        return potted->_after;
    }

    auto ballCollision = toBallCollision();
    if(ballCollision) {
        return ballCollision->_after;
    }

    auto ballRailCollision = toBallRailCollision();
    if(ballRailCollision) {
        return ballRailCollision->_after;
    }

    auto ballMoving = toBallMoving();
    if(ballMoving) {
        return ballMoving->_after;
    }

    auto ballShot = toBallShot();
    if(ballShot) {
        return ballShot->_ball;
    }

    return std::nullopt;
}

std::pair<std::string, std::string> billiard::search::node::Node::getInvolvedId() const {
    auto inRest = toInRest();
    if (inRest) {
        return std::pair<std::string, std::string>{"", ""};
    }

    auto potted = toPotted();
    if(potted) {
        return std::pair<std::string, std::string>{"", ""};
    }

    auto ballCollision = toBallCollision();
    if(ballCollision) {
        return std::pair<std::string, std::string>{ballCollision->_cause._ball1, ballCollision->_cause._ball2};
    }

    auto ballRailCollision = toBallRailCollision();
    if(ballRailCollision) {
        return std::pair<std::string, std::string>{"", ""};
    }

    auto ballMoving = toBallMoving();
    if(ballMoving) {
        return std::pair<std::string, std::string>{"", ""};
    }

    auto ballShot = toBallShot();
    if(ballShot) {
        return std::pair<std::string, std::string>{"", ""};
    }

    return std::pair<std::string, std::string>{"", ""};
}

bool billiard::search::node::Node::isStatic() const {
    return _type == NodeType::BALL_IN_REST || _type == NodeType::BALL_POTTING;
}

billiard::search::node::Layer::Layer(float time, std::unordered_map<std::string, Node> nodes) :
    _time(time),
    _dynamic(getDynamic(nodes)),
    _static(getStatic(nodes)),
    _nodes(std::move(nodes)),
    _isFirst(false),
    _isLast(false) {
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
            billiard::search::node::NodeType::BALL_SHOT,
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

const billiard::search::node::Layer& billiard::search::node::System::firstLayer() const {
    return _layers.at(0);
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

void billiard::search::node::System::append(Layer layer) {
    layer._isLast = true;
    float deltaTime;
    if (_layers.empty()) {
        layer._isFirst = true;
        deltaTime = layer._time;
    } else {
        _layers.at(_layers.size() - 1)._isLast = false;
        deltaTime = layer._time - _layers.at(_layers.size() - 1)._time;
    }

    for(auto& node : layer._nodes) {

        if (_nextRolling.find(node.first) != _nextRolling.end()) {
            auto nextRollingEvent = _nextRolling.find(node.first)->second - deltaTime;
            _nextRolling.erase(node.first);
            _nextRolling.insert({node.first, std::max(0.0f, nextRollingEvent)});
        }

        if (node.second._type == NodeType::BALL_COLLISION) {
            _lastRailCollisions.erase(node.first);

            // Wenn Kugel nicht rollt, dann gleitet sie bei einer Kollision.
            auto after = node.second.after();
            static glm::vec2 zero {0, 0};
            if (!after->_isRolling && after->_velocity != zero) {
                _nextRolling.erase(node.first);
                _nextRolling.insert({node.first, billiard::physics::timeToRolling(after->_velocity)});
            } else {
                _nextRolling.erase(node.first);
            }
        } else if (node.second._type == NodeType::BALL_SHOT) {
            // Kugel wurde mit cue stick angestossen, sie gleitet zuerst und wechselt nach einer gewissen Zeit zu rollen.
            auto after = node.second.after();
            _nextRolling.erase(node.first);
            _nextRolling.insert({node.first, billiard::physics::timeToRolling(after->_velocity)});
        } else if (node.second._type == NodeType::BALL_IN_REST) {
            // Kugel ist stehengeblieben, sie kann nicht mehr von gleiten zu rollen wechseln.
            _nextRolling.erase(node.first);
        } else if (node.second._type == NodeType::BALL_POTTING) {
            // Kugel wurde eingelocht, sie kann nicht mehr von gleiten zu rollen wechseln.
            _nextRolling.erase(node.first);
        } else if (node.second._type == NodeType::BALL_MOVING) {
            auto after = node.second.after();
            if (after->_isRolling) {
                // Kugel rollt bereits, sie kann nicht mehr von gleiten zu rollen wechseln.
                _nextRolling.erase(node.first);
            }
        }

        auto asRailCollision = node.second.toBallRailCollision();
        if (asRailCollision) {
            _lastRailCollisions.erase(node.first);
            _lastRailCollisions.insert({node.first, asRailCollision->_cause._rail});

            // Bei einer Bandenkollision rollt die Kugel aufgrund der Eigenschaft der Bandenhöhe weiter.
            _nextRolling.erase(node.first);
        }
    }

    _layers.emplace_back(std::move(layer));
}
