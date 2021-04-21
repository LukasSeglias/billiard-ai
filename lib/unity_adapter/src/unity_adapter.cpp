#include <unity_adapter/unity_adapter.hpp>
#include <fstream>
#include <filesystem>
#include "event_queue.hpp"
#include <memory>

#include <string>

std::shared_ptr<RootObject> _currentState;
Debugger _debugger;

///////////////////////////////////////////////////////
//// Event-Queues
///////////////////////////////////////////////////////
using AnimationChangedEventQueue = unity::EventQueue<RootObject, AnimationChangedEventCallback>;
AnimationChangedEventQueue* _animationChangedEventQueue = nullptr;

///////////////////////////////////////////////////////
//// Implementation
///////////////////////////////////////////////////////
std::shared_ptr<RootObject> testState();

void onStart() {
    _currentState = testState();
    _animationChangedEventQueue = new AnimationChangedEventQueue{};
}

void onTearDown() {
    delete _animationChangedEventQueue;
    _animationChangedEventQueue = nullptr;
}

void processEvents() {
    _animationChangedEventQueue->process();
}

void onAnimationChangedEvent(AnimationChangedEventCallback callback) {
    _animationChangedEventQueue->callback(callback);
}

void configuration(Configuration config) {
    _debugger((std::string("radius: ") + std::to_string(config.radius)).c_str());
    _debugger((std::string("width: ") + std::to_string(config.width)).c_str());
    _debugger((std::string("height: ") + std::to_string(config.height)).c_str());
}

void capture() {
    #ifdef NDEBUG
        // TODO: Capture state and update _currentState
    #endif

    _animationChangedEventQueue->push(_currentState);  // TODO: Delete this
}

void search(Search search) {
    // TODO: Map _currentState to internal type of billiard_search
    // TODO: Search with current capturing
    // TODO: Update _currentState

    _animationChangedEventQueue->push(_currentState);  // TODO: Delete this
}

void debugger(Debugger debugger) {
    _debugger = debugger;
}

std::shared_ptr<RootObject> map(const State& state);

void state(State state) {
    #ifndef NDEBUG
        _currentState = map(state);
    #endif
}

Ball* map(const BallState* ballState, int ballSize);

std::shared_ptr<RootObject> map(const State& state) {
    KeyFrame keyFrame{0.0, map(state.balls, state.ballSize), state.ballSize};

    KeyFrame keyFrames[] = {
            keyFrame,
            keyFrame
    };
    AnimationModel models[] = {AnimationModel{keyFrames, 2}};
    return std::make_shared<RootObject>(models, 1);
}

Ball* map(const BallState* ballState, int ballSize) {
    Ball* balls = new Ball[ballSize];

    for (int i = 0; i < ballSize; i++) {
        balls[i] = Ball{ballState[i].type, ballState[i].id, ballState[i].position, Vec2{0, 0}, true};
    }
    return balls;
}

// TODO: Delete
std::shared_ptr<RootObject> testState() {
    Ball balls1[] = {
            Ball {
                    "WHITE",
                    "WHITE",
                    Vec2 {-0.5, 0.5},
                    Vec2 {0.172627, -0.172627},
                    true
            },
            Ball {
                    "RED",
                    "RED_1",
                    Vec2 {-0.2, 0.2},
                    Vec2 {0, 0},
                    true
            },
            Ball {
                    "RED",
                    "RED_2",
                    Vec2 {0.2, 0.2},
                    Vec2 {0, 0},
                    true
            }
    };

    Ball balls2[] = {
            Ball {
                    "WHITE",
                    "WHITE",
                    Vec2 {-0.23677, 0.23677},
                    Vec2 {0.090603, -0.090603},
                    true
            },
            Ball {
                    "RED",
                    "RED_1",
                    Vec2 {-0.2, 0.2},
                    Vec2 {0, 0},
                    true
            },
            Ball {
                    "RED",
                    "RED_2",
                    Vec2 {0.2, 0.2},
                    Vec2 {0, 0},
                    false
            }
    };

    Ball balls3[] = {
            Ball {
                    "WHITE",
                    "WHITE",
                    Vec2 {-0.23677, 0.23677},
                    Vec2 {0, 0},
                    true
            },
            Ball {
                    "RED",
                    "RED_1",
                    Vec2 {-0.2, 0.2},
                    Vec2 {0.090603, -0.090603},
                    true
            },
            Ball {
                    "RED",
                    "RED_2",
                    Vec2 {0.2, 0.2},
                    Vec2 {0, 0},
                    false
            }
    };

    Ball balls4[] = {
            Ball {
                    "WHITE",
                    "WHITE",
                    Vec2 {-0.23677, 0.23677},
                    Vec2 {0, 0},
                    true
            },
            Ball {
                    "RED",
                    "RED_1",
                    Vec2 {-0.099921, 0.099921},
                    Vec2 {0, 0},
                    false
            },
            Ball {
                    "RED",
                    "RED_2",
                    Vec2 {0.2, 0.2},
                    Vec2 {0, 0},
                    false
            }
    };

    Ball balls5[] = {
            Ball {
                    "WHITE",
                    "WHITE",
                    Vec2 {-0.23677, 0.23677},
                    Vec2 {0, 0},
                    true
            },
            Ball {
                    "RED",
                    "RED_1",
                    Vec2 {-0.099921, 0.099921},
                    Vec2 {0, 0},
                    false
            },
            Ball {
                    "RED",
                    "RED_2",
                    Vec2 {0.2, 0.2},
                    Vec2 {0, 0},
                    true
            }
    };

    Ball balls6[] = {
            Ball {
                    "WHITE",
                    "WHITE",
                    Vec2 {-0.23677, 0.23677},
                    Vec2 {0, 0},
                    true
            },
            Ball {
                    "RED",
                    "RED_1",
                    Vec2 {-0.099921, 0.099921},
                    Vec2 {0, 0},
                    false
            },
            Ball {
                    "RED",
                    "RED_2",
                    Vec2 {0.2, 0.2},
                    Vec2 {0, 0},
                    false
            }
    };

    KeyFrame keyFrames[6] = {
            KeyFrame {
                0,
                balls1,
                3
            },
            KeyFrame {
                2,
                balls2,
                3
            },
            KeyFrame {
                2,
                balls3,
                3
            },
            KeyFrame {
                4.20918,
                balls4,
                3
            },
            KeyFrame {
                4.20918,
                balls5,
                3
            },
            KeyFrame {
                4.3,
                balls6,
                3
            }
    };

    AnimationModel models[] = {AnimationModel{keyFrames, 6}};
    return std::make_shared<RootObject>(models, 1);
}