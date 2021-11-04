#include <unity_adapter/type.hpp>

bool Vec2::operator!=(const Vec2& other) const {
    return x != other.x || y != other.y;
}

Ball::Ball() : type(), id(), position(), velocity(), visible() {}

Ball::Ball(const char* type, const char* id, Vec2 position, Vec2 velocity, bool visible) :
                                                                        type(_strdup(type)),
                                                                        id(_strdup(id)),
                                                                         position(position),
                                                                         velocity(velocity), visible(visible) {
}

Ball::Ball(Ball&& other) noexcept :
        type(other.type),
        id(other.id),
        position(other.position),
        velocity(other.velocity),
        visible(other.visible) {
    other.type = nullptr;
    other.id = nullptr;
}

Ball::Ball(const Ball& other) noexcept:
        type(_strdup(other.type)),
        id(_strdup(other.id)),
        position(other.position),
        velocity(other.velocity),
        visible(other.visible) {
}

Ball& Ball::operator=(Ball&& other) noexcept {
    type = other.type;
    id = other.id;
    position = other.position;
    velocity = other.velocity;
    visible = other.visible;
    other.type = nullptr;
    other.id = nullptr;
    return *this;
}

Ball& Ball::operator=(const Ball& other) noexcept {
    if (&other == this) {
        return *this;
    }
    type = _strdup(other.type);
    id = _strdup(other.id);
    position = other.position;
    velocity = other.velocity;
    visible = other.visible;
    return *this;
}
Ball::~Ball() {
    delete id;
    id = nullptr;
    delete type;
    type = nullptr;
}

KeyFrame::KeyFrame() : time(0), ballSize(0), balls(nullptr), firstFrame(false) {
}

KeyFrame::KeyFrame(double time, Ball balls[], int ballSize, bool firstFrame) : time(time), ballSize(ballSize),
                                                                               balls(nullptr), firstFrame(firstFrame) {
    this->balls = new Ball[ballSize];
    for (int i = 0; i < ballSize; i++) {
        this->balls[i] = Ball{std::move(balls[i])};
    }
}

KeyFrame::KeyFrame(const KeyFrame& other) noexcept: time(other.time), ballSize(other.ballSize), balls(nullptr),
                                                    firstFrame(other.firstFrame) {
    this->balls = new Ball[ballSize];
    for (int i = 0; i < ballSize; i++) {
        this->balls[i] = Ball{other.balls[i]};
    }
}

KeyFrame::KeyFrame(KeyFrame&& other) noexcept: time(other.time), ballSize(other.ballSize), balls(other.balls),
                                               firstFrame(other.firstFrame) {
    other.balls = nullptr;
    other.ballSize = 0;
}

KeyFrame& KeyFrame::operator=(KeyFrame&& other) noexcept {
    time = other.time;
    ballSize = other.ballSize;
    balls = other.balls;
    other.balls = nullptr;
    other.ballSize = 0;
    firstFrame = other.firstFrame;
    return *this;
}

KeyFrame& KeyFrame::operator=(const KeyFrame& other) noexcept {
    if (&other == this) {
        return *this;
    }
    time = other.time;
    ballSize = other.ballSize;
    this->balls = new Ball[ballSize];
    for (int i = 0; i < ballSize; i++) {
        this->balls[i] = Ball{other.balls[i]};
    }
    firstFrame = other.firstFrame;

    return *this;
}

KeyFrame::~KeyFrame() {
    delete[] balls;
    balls = nullptr;
}

AnimationModel::AnimationModel() : keyFrameSize(0), keyFrames(nullptr) {
}

AnimationModel::AnimationModel(KeyFrame keyFrames[], int keyFrameSize) : keyFrameSize(keyFrameSize), keyFrames(nullptr) {
    this->keyFrames = new KeyFrame[keyFrameSize];
    for(int i = 0; i < keyFrameSize; i++) {
        this->keyFrames[i] = KeyFrame{std::move(keyFrames[i])};
    }
}

AnimationModel::AnimationModel(const AnimationModel& other) noexcept : keyFrameSize(other.keyFrameSize), keyFrames(nullptr) {
    this->keyFrames = new KeyFrame[keyFrameSize];
    for(int i = 0; i < keyFrameSize; i++) {
        this->keyFrames[i] = KeyFrame{other.keyFrames[i]};
    }
}

AnimationModel::AnimationModel(AnimationModel&& other) noexcept : keyFrameSize(other.keyFrameSize), keyFrames(other.keyFrames) {
    other.keyFrames = nullptr;
    other.keyFrameSize = 0;
}

AnimationModel& AnimationModel::operator=(AnimationModel&& other) noexcept {
    keyFrameSize = other.keyFrameSize;
    keyFrames = other.keyFrames;
    other.keyFrames = nullptr;
    other.keyFrameSize = 0;
    return *this;
}

AnimationModel& AnimationModel::operator=(const AnimationModel& other) noexcept {
    if (&other == this) {
        return *this;
    }

    keyFrameSize = other.keyFrameSize;
    this->keyFrames = new KeyFrame[keyFrameSize];
    for(int i = 0; i < keyFrameSize; i++) {
        this->keyFrames[i] = KeyFrame{other.keyFrames[i]};
    }
    return *this;
}

AnimationModel::~AnimationModel() {
    delete[] keyFrames;
    keyFrames = nullptr;
}

RootObject::RootObject(AnimationModel* animations, int animationSize) : animations(nullptr), animationSize(animationSize) {
        this->animations = new AnimationModel[animationSize];
        for(int i = 0; i < animationSize; i++) {
            this->animations[i] = AnimationModel{std::move(animations[i])};
        }
}

RootObject::RootObject(RootObject&& other) noexcept : animations(other.animations), animationSize(other.animationSize) {
    other.animations = nullptr;
    other.animationSize = 0;
}

RootObject::RootObject(const RootObject& other) noexcept : animations(nullptr), animationSize(other.animationSize) {
    this->animations = new AnimationModel[animationSize];
    for(int i = 0; i < animationSize; i++) {
        this->animations[i] = AnimationModel{other.animations[i]};
    }
}

RootObject& RootObject::operator=(RootObject&& other) noexcept {
    animations = other.animations;
    animationSize = other.animationSize;
    other.animations = nullptr;
    other.animationSize = 0;
    return *this;
}

RootObject& RootObject::operator=(const RootObject& other) noexcept {
    if (&other == this) {
        return *this;
    }

    animationSize = other.animationSize;
    this->animations = new AnimationModel[animationSize];
    for(int i = 0; i < animationSize; i++) {
        this->animations[i] = AnimationModel{other.animations[i]};
    }
    return *this;
}

RootObject::~RootObject() {
    delete[] animations;
    animations = nullptr;
}

BallState::BallState() :
    type(nullptr),
    id(nullptr),
    position(),
    fromUnity(false) {
}

BallState::BallState(const char* type, const char* id, Vec2 position, bool fromUnity) :
    type(_strdup(type)),
    id(_strdup(id)),
    position(position),
    fromUnity(fromUnity) {
}

BallState::BallState(BallState&& other) noexcept :
    type(other.type),
    id(other.id),
    position(other.position),
    fromUnity(other.fromUnity) {
    other.type = nullptr;
    other.id = nullptr;
}

BallState::BallState(const BallState& other) noexcept :
    type(_strdup(other.type)),
    id(_strdup(other.id)),
    position(other.position),
    fromUnity(other.fromUnity) {
}

BallState& BallState::operator=(BallState&& other) noexcept {
    type = other.type;
    id = other.id;
    position = other.position;
    fromUnity = other.fromUnity;
    other.type = nullptr;
    other.id = nullptr;

    return *this;
}

BallState& BallState::operator=(const BallState& other) noexcept {
    if (&other == this) {
        return *this;
    }

    type = _strdup(other.type);
    id = _strdup(other.id);
    position = other.position;
    fromUnity = other.fromUnity;

    return *this;
}

BallState::~BallState() {
    if (!fromUnity) {
        delete type;
        type = nullptr;
        delete id;
        id = nullptr;
    }
}

State::State() :
    balls(nullptr),
    ballSize(0),
    velocity({0, 0}),
    fromUnity(false) {
}

State::State(BallState* balls, int ballSize, Vec2 velocity, bool fromUnity) :
    balls(balls),
    ballSize(ballSize),
    velocity(velocity),
    fromUnity(fromUnity) {
}

State::State(State&& other) noexcept :
    balls(other.balls),
    ballSize(other.ballSize),
    velocity(other.velocity),
    fromUnity(other.fromUnity) {
    other.balls = nullptr;
    other.ballSize = 0;
}

State::State(const State& other) noexcept :
    balls(new BallState[other.ballSize]),
    ballSize(other.ballSize),
    velocity(other.velocity),
    fromUnity(other.fromUnity) {
    for(int i = 0; i < ballSize; i++) {
        this->balls[i] = BallState{other.balls[i]};
    }
}

State& State::operator=(State&& other) noexcept {
    balls = other.balls;
    ballSize = other.ballSize;
    velocity = other.velocity;
    fromUnity = other.fromUnity;
    other.balls = nullptr;
    other.ballSize = 0;

    return *this;
}

State& State::operator=(const State& other) noexcept {
    if (&other == this) {
        return *this;
    }

    ballSize = other.ballSize;
    velocity = other.velocity;
    fromUnity = other.fromUnity;
    this->balls = new BallState[ballSize];
    for(int i = 0; i < ballSize; i++) {
        this->balls[i] = BallState{other.balls[i]};
    }
    return *this;
}

State::~State() {
    if (!fromUnity) {
        delete[] balls;
        balls = nullptr;
    }
}
