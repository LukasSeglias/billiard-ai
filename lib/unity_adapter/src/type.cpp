#include <unity_adapter/type.hpp>

Ball::Ball() : type(), id(), position(), velocity(), visible() {}

Ball::Ball(char* type, char* id, Vec2 position, Vec2 velocity, bool visible) : type(_strdup(type)), id(_strdup(id)),
                                                                         position(position),
                                                                         velocity(velocity), visible(visible) {
}

Ball::Ball(Ball&& other) noexcept :
        type(_strdup(other.type)),
        id(_strdup(other.id)),
        position(other.position),
        velocity(other.velocity),
        visible(other.visible) {
    other.type = nullptr;
    other.id = nullptr;
}

Ball::Ball(const Ball& other) noexcept : type(_strdup(other.type)), id(_strdup(other.id)), position(other.position),
                                   velocity(other.velocity),
                                   visible(other.visible) {
}

Ball& Ball::operator=(Ball&& other) noexcept {
    type = _strdup(other.type);
    id = _strdup(other.id);
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
}

KeyFrame& KeyFrame::operator=(KeyFrame&& other) noexcept {
    time = other.time;
    ballSize = other.ballSize;
    balls = other.balls;
    other.balls = nullptr;
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
}

AnimationModel& AnimationModel::operator=(AnimationModel&& other) noexcept {
    keyFrameSize = other.keyFrameSize;
    keyFrames = other.keyFrames;
    other.keyFrames = nullptr;
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