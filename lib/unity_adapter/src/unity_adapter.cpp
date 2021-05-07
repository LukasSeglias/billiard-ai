#include <unity_adapter/unity_adapter.hpp>
#include "event_queue.hpp"
#include <memory>
#include <string>
#include <billiard_detection/billiard_detection.hpp>

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

inline cv::Point3d toPoint3d(const Vec3& vec) {
    return cv::Point3d { vec.x, vec.y, vec.z };
}

inline cv::Point3f toPoint3f(const Vec3& vec) {
    return cv::Point3f { (float)vec.x, (float)vec.y, (float)vec.z };
}

inline cv::Vec3d toVec3d(const Vec3& vec) {
    return cv::Vec3d { vec.x, vec.y, vec.z };
}

inline billiard::detection::CameraIntrinsics toIntrinsics(const CameraIntrinsics& camera);
cv::Ptr<cv::aruco::Board> createArucoBoard(const ArucoMarkers& markers);
inline billiard::detection::Plane toPlane(const Plane& plane);
inline billiard::detection::WorldToModelCoordinates toWorldToModelCoordinates(const WorldToModel& worldToModel);

void configuration(Configuration config) {
    // TODO: Map configuration and pass it to the library, delete all debugger outputs
    _debugger((std::string("radius: ") + std::to_string(config.radius)).c_str());
    _debugger((std::string("width: ") + std::to_string(config.width)).c_str());
    _debugger((std::string("height: ") + std::to_string(config.height)).c_str());
    _debugger((std::string("segments: ") + std::to_string(config.segmentSize)).c_str());
    _debugger((std::string("targets: ") + std::to_string(config.targetSize)).c_str());

    _debugger((std::string("marker pattern size: ") + std::to_string(config.markers.patternSize) + ", side length: " + std::to_string(config.markers.sideLength) + "mm").c_str());
    _debugger((std::string("world to model") + " translation is (" + std::to_string(config.worldToModel.translation.x) + ", " + std::to_string(config.worldToModel.translation.y) + ", " + std::to_string(config.worldToModel.translation.z) + ")").c_str());
    _debugger((std::string("marker 0") + " is at (" + std::to_string(config.markers.m0.x) + ", " + std::to_string(config.markers.m0.y) + ", " + std::to_string(config.markers.m0.z) + ")").c_str());
    _debugger((std::string("marker 1") + " is at (" + std::to_string(config.markers.m1.x) + ", " + std::to_string(config.markers.m1.y) + ", " + std::to_string(config.markers.m1.z) + ")").c_str());
    _debugger((std::string("marker 2") + " is at (" + std::to_string(config.markers.m2.x) + ", " + std::to_string(config.markers.m2.y) + ", " + std::to_string(config.markers.m2.z) + ")").c_str());
    _debugger((std::string("marker 3") + " is at (" + std::to_string(config.markers.m3.x) + ", " + std::to_string(config.markers.m3.y) + ", " + std::to_string(config.markers.m3.z) + ")").c_str());
    _debugger((std::string("ball plane") + " is: (p - " + "(" + std::to_string(config.ballPlane.point.x) + ", " + std::to_string(config.ballPlane.point.y) + ", " + std::to_string(config.ballPlane.point.z) + ")" + ") * " + "(" + std::to_string(config.ballPlane.normal.x) + ", " + std::to_string(config.ballPlane.normal.y) + ", " + std::to_string(config.ballPlane.normal.z) + ")" + " = 0").c_str());
    _debugger((std::string("camera intrinsics:")
                + " f: " + "(" + std::to_string(config.camera.fx) + ", " + std::to_string(config.camera.fy) + ")"
                + " c: " + "(" + std::to_string(config.camera.cx) + ", " + std::to_string(config.camera.cy) + ")"
                + " skew: " + std::to_string(config.camera.skew)
                + " k: " + "(" + std::to_string(config.camera.k1) + ", " + std::to_string(config.camera.k2) + ", " + std::to_string(config.camera.k3) + ")"
                + " p: " + "(" + std::to_string(config.camera.p1) + ", " + std::to_string(config.camera.p2) + ")"
                ).c_str());

    for(int i = 0; i < config.segmentSize; i++) {
        auto& segment = config.segments[i];
        _debugger((std::string("rail segment [") + std::to_string(i) + "] has start at [" + std::to_string(segment.start.x) + ";" + std::to_string(segment.start.y) + "] and end at [" + std::to_string(segment.end.x) + ";" + std::to_string(segment.end.y) + "]").c_str());
    }

    for(int i = 0; i < config.targetSize; i++) {
        auto& target = config.targets[i];
        _debugger((std::string("target [") + std::to_string(i) + "] is located at [" + std::to_string(target.position.x) + ";" + std::to_string(target.position.y) + "] and has radius " + std::to_string(target.radius)).c_str());
    }

    billiard::detection::CameraIntrinsics intrinsics = toIntrinsics(config.camera);
    _debugger("CameraIntrinsics created");

    billiard::detection::Plane ballPlane = toPlane(config.ballPlane);
    _debugger("BallPlane created");

    cv::Ptr<cv::aruco::Board> board = createArucoBoard(config.markers);
    _debugger("Aruco-board created");

    billiard::detection::WorldToModelCoordinates worldToModel = toWorldToModelCoordinates(config.worldToModel);
    _debugger("worldToModel created");

    _debugger("All configuration mapped");
}

inline billiard::detection::WorldToModelCoordinates toWorldToModelCoordinates(const WorldToModel& worldToModel) {
    return billiard::detection::WorldToModelCoordinates {
            toVec3d(worldToModel.translation)
    };
}

inline billiard::detection::Plane toPlane(const Plane& plane) {
    return billiard::detection::Plane {
            toVec3d(plane.point),
            toVec3d(plane.normal)
    };
}

inline billiard::detection::CameraIntrinsics toIntrinsics(const CameraIntrinsics& camera) {
    return billiard::detection::CameraIntrinsics {
            cv::Point2d { camera.fx, camera.fy },
            cv::Point2d { camera.cx, camera.cy },
            camera.skew,
            cv::Point3d { camera.k1, camera.k2, camera.k3 },
            cv::Point2d { camera.p1, camera.p2 },
            cv::Point2d { camera.sx, camera.sy }
    };
}

cv::Ptr<cv::aruco::Board> createArucoBoard(const ArucoMarkers& markers) {

    const int nMarkers = 4;
    std::vector<int> ids = {0, 1, 2, 3};
    cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::generateCustomDictionary(nMarkers, markers.patternSize);

    const float side = markers.sideLength;

    auto cornerPositions = [&side](const cv::Point3f& bottomLeft) -> std::vector<cv::Point3f> {
        return {
                bottomLeft + cv::Point3f{ 0, side, 0 },
                bottomLeft + cv::Point3f{ side, side, 0 },
                bottomLeft + cv::Point3f{ side, 0, 0 },
                bottomLeft + cv::Point3f{ 0, 0, 0 },
        };
    };

    cv::Point3f centerOffset{side/2, side/2, 0};
    std::vector<std::vector<cv::Point3f>> objPoints = {
            cornerPositions(toPoint3f(markers.m0) - centerOffset), // Marker 0
            cornerPositions(toPoint3f(markers.m1) - centerOffset), // Marker 1
            cornerPositions(toPoint3f(markers.m2) - centerOffset), // Marker 2
            cornerPositions(toPoint3f(markers.m3) - centerOffset), // Marker 3
    };

    cv::Ptr<cv::aruco::Board> board = cv::aruco::Board::create(objPoints, dictionary, ids);
    return board;
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