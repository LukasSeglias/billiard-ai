#include <billiard_detection/type.hpp>

bool billiard::detection::Ball::operator==(const Ball& other) const {
    return this->_position == other._position &&
            this->_type == other._type &&
            this->_id == other._id;
}

bool billiard::detection::State::operator==(const State& other) const {
    return this->_balls == other._balls;
}