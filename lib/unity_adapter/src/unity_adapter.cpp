#include <unity_adapter/unity_adapter.hpp>
#include "event_queue.hpp"
#include <memory>
#include <string>

Debugger _debugger;

///////////////////////////////////////////////////////
//// Event-Queues
///////////////////////////////////////////////////////
using AnimationChangedEventQueue = unity::EventQueue<RootObject, AnimationChangedEventCallback>;
AnimationChangedEventQueue* _animationChangedEventQueue = nullptr;

///////////////////////////////////////////////////////
//// Mappings - Declaration
///////////////////////////////////////////////////////
std::shared_ptr<RootObject> map(const State& state); // Unity - CPP
// TODO: Add mapping from external config to internal config (Apply config)
// TODO: Add mapping from internal state to external animation (Capture)
// TODO: Add mapping from internal animation to external animation (Search)

///////////////////////////////////////////////////////
//// Implementation
///////////////////////////////////////////////////////
std::shared_ptr<RootObject> _currentState;
std::shared_ptr<RootObject> testState(); // TODO: Delete

void onStart() {
    _animationChangedEventQueue = new AnimationChangedEventQueue{};
    _currentState = testState(); // TODO: Delete
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
    // TODO: Map configuration and pass it to the library, delete all debugger outputs
    _debugger((std::string("radius: ") + std::to_string(config.radius)).c_str());
    _debugger((std::string("width: ") + std::to_string(config.width)).c_str());
    _debugger((std::string("height: ") + std::to_string(config.height)).c_str());
    _debugger((std::string("segments: ") + std::to_string(config.segmentSize)).c_str());
    _debugger((std::string("targets: ") + std::to_string(config.targetSize)).c_str());
    _debugger((std::string("markers: ") + std::to_string(config.markerSize)).c_str());

    for(int i = 0; i < config.segmentSize; i++) {
        auto& segment = config.segments[i];
        _debugger((std::string("rail segment [") + std::to_string(i) + "] has start at [" + std::to_string(segment.start.x) + ";" + std::to_string(segment.start.y) + "] and end at [" + std::to_string(segment.end.x) + ";" + std::to_string(segment.end.y) + "]").c_str());
    }

    for(int i = 0; i < config.targetSize; i++) {
        auto& target = config.targets[i];
        _debugger((std::string("target [") + std::to_string(i) + "] is located at [" + std::to_string(target.position.x) + ";" + std::to_string(target.position.y) + "] and has radius " + std::to_string(target.radius)).c_str());
    }

    for(int i = 0; i < config.markerSize; i++) {
        auto& marker = config.markers[i];
        _debugger((std::string("marker [") + std::to_string(i) + "] is located at [" + std::to_string(marker.position.x) + ";" + std::to_string(marker.position.y) + "] and has side length " + std::to_string(marker.sideLength)).c_str());
    }
}

void capture() {
    // TODO: Capture state and update _currentState
    // TODO: Push _currentState to the animationChangedEventQueue
    // TODO: Hint, write a mapping function according to the existing "map" to create an animation from a state.

    _animationChangedEventQueue->push(_currentState);
}

void search(Search search) {
    // TODO: Capture state
    // TODO: Start search
    // TODO: When search finished, the result has to be mapped and pushed to _animationChangedEventQueue

    _animationChangedEventQueue->push(_currentState); // TODO: Delete
}

void debugger(Debugger debugger) {
    _debugger = debugger;
}

void state(State state) {
    #ifndef NDEBUG
        _currentState = map(state);
        _animationChangedEventQueue->push(_currentState);
    #endif
}

///////////////////////////////////////////////////////
//// Mappings - Implementation
///////////////////////////////////////////////////////

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

// TODO: Add mapping from internal state to external animation (Capture)
// TODO: Add mapping from internal animation to external animation (Search)

// TODO: Delete all after this comment
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