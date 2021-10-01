#include <unity_adapter/unity_adapter.hpp>
#include "event_queue.hpp"
#include <memory>
#include <string>
#include <billiard_detection/billiard_detection.hpp>
#include <billiard_snooker/billiard_snooker.hpp>
#include <billiard_capture/billiard_capture.hpp>

Debugger _debugger;

bool LIVE = true;
std::string IMAGE_PATH = "D:\\Dev\\billiard-ai\\cmake-build-debug\\test\\billiard_detection\\resources\\test_detection\\1_scaled_HD.png";

///////////////////////////////////////////////////////
//// Event-Queues
///////////////////////////////////////////////////////
using AnimationChangedEventQueue = unity::EventQueue<RootObject, AnimationChangedEventCallback>;
AnimationChangedEventQueue* _animationChangedEventQueue = nullptr;

///////////////////////////////////////////////////////
//// Mappings - Declaration
///////////////////////////////////////////////////////
std::shared_ptr<RootObject> map(const State& state); // Unity - CPP
std::shared_ptr<RootObject> map(const billiard::detection::State& state); // Unity - CPP
// TODO: Add mapping from external config to internal config (Apply config)
// TODO: Add mapping from internal animation to external animation (Search)

///////////////////////////////////////////////////////
//// Implementation
///////////////////////////////////////////////////////
std::shared_ptr<billiard::detection::StateTracker> stateTracker;
std::shared_ptr<billiard::capture::CameraCapture> cameraCapture;
std::shared_ptr<billiard::detection::DetectionConfig> detectionConfig;
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
inline billiard::detection::ArucoMarkers createArucoMarkers(const ArucoMarkers& input);
inline billiard::detection::Table toTable(const Table& table);

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

    billiard::detection::ArucoMarkers markers = createArucoMarkers(config.markers);
    _debugger("Aruco-board created");
    _debugger((std::string("Markers:")
               + " patternSize: " + std::to_string(markers.patternSize)
               + " sideLength: " + std::to_string(markers.sideLength)
               + " bottomLeft: " + "" + std::to_string(markers.bottomLeft.x) + ", " + std::to_string(markers.bottomLeft.y) + ", " + std::to_string(markers.bottomLeft.z)
               + " bottomRight: " + "" + std::to_string(markers.bottomRight.x) + ", " + std::to_string(markers.bottomRight.y) + ", " + std::to_string(markers.bottomRight.z)
               + " topLeft: " + "" + std::to_string(markers.topLeft.x) + ", " + std::to_string(markers.topLeft.y) + ", " + std::to_string(markers.topLeft.z)
               + " topRight: " + "" + std::to_string(markers.topRight.x) + ", " + std::to_string(markers.topRight.y) + ", " + std::to_string(markers.topRight.z)
              ).c_str());

    billiard::detection::Table table = toTable(config.table);
    _debugger((std::string("Table: ")
                + " innerTableLength: " + std::to_string(table.innerTableLength)
                + " innerTableWidth: " + std::to_string(table.innerTableWidth)
                + " ballDiameter: " + std::to_string(table.ballDiameter)
                + " arucoHeightAboveInnerTable: " + std::to_string(table.arucoHeightAboveInnerTable)
                + " railWorldPointZComponent: " + std::to_string(table.railWorldPointZComponent)
                + " worldToRail: " + std::to_string(table.worldToRail[0]) + ", " + std::to_string(table.worldToRail[1]) + ", " + std::to_string(table.worldToRail[2])
                ).c_str());

    if (cameraCapture) {
        cameraCapture->close();
    }

    cv::Mat image;
    if (LIVE) {
        cameraCapture = std::make_shared<billiard::capture::CameraCapture>();
        if (cameraCapture->open()) {
            _debugger("[SUCCESS]: Opened camera capture");
        } else {
            _debugger("[ERROR]: Unable to open camera capture");
            return;
        }

        int skippedFrames = 30;
        while (skippedFrames-- > 0) cameraCapture->read();
        billiard::capture::CameraFrames frames = cameraCapture->read();
        image = frames.color;
    } else {
        image = cv::imread(IMAGE_PATH);
        cv::imshow("image", image);
    }

    detectionConfig = std::make_shared<billiard::detection::DetectionConfig>(billiard::detection::configure(image, table, markers, intrinsics));
    if (detectionConfig->valid) {
        _debugger("[SUCCESS]: Detection configured");
    } else {
        _debugger("[ERROR]: Unable to configure detection");
        return;
    }

    if (billiard::snooker::configure(*detectionConfig)) {
        _debugger("[SUCCESS]: Snooker detection configured");
    } else {
        _debugger("[ERROR]: Unable to configure snooker detection");
        return;
    }

    if (LIVE) {
        stateTracker = std::make_shared<billiard::detection::StateTracker>(cameraCapture,
                                                                           detectionConfig,
                                                                           billiard::snooker::detect,
                                                                           [](const billiard::detection::State& previousState,
                                                                              billiard::detection::State& currentState,
                                                                              const cv::Mat& image) {
            billiard::snooker::classify(previousState, currentState, image);
        });
    }

    _debugger("All configuration mapped");
}

inline billiard::detection::Table toTable(const Table& table) {
    return billiard::detection::Table {
            table.innerTableLength,
            table.innerTableWidth,
            table.ballDiameter,
            table.arucoHeightAboveInnerTable,
            table.railWorldPointZComponent,
            toVec3d(table.worldToRail)
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

inline billiard::detection::ArucoMarkers createArucoMarkers(const ArucoMarkers& input) {

    billiard::detection::ArucoMarkers markers;
    markers.patternSize = input.patternSize;
    markers.sideLength  = input.sideLength;
    markers.bottomLeft  = toPoint3f(input.m0);
    markers.bottomRight = toPoint3f(input.m1);
    markers.topRight    = toPoint3f(input.m2);
    markers.topLeft     = toPoint3f(input.m3);
    return markers;
}

inline void printState(const billiard::detection::State& state) {
    _debugger(("[Detection]: Detected " + std::to_string(state._balls.size()) + " balls").c_str());
    for (auto& ball : state._balls) {
        _debugger(("[Detection]: Ball: " + ball._id + " " + ball._type + " at (" + std::to_string(ball._position[0]) + ", " + std::to_string(ball._position[1]) + ")").c_str());
    }
}

void capture() {
    _debugger("[COMMAND]: Capture");
    // TODO: Capture state and update _currentState
    // TODO: Push _currentState to the animationChangedEventQueue
    // TODO: Hint, write a mapping function according to the existing "map" to create an animation from a state.

    if (LIVE) {
        if (stateTracker) {

            auto stateFuture = stateTracker->capture();
            stateFuture.wait();
            auto state = stateFuture.get();
            printState(state);

            _currentState = map(state);
        }
    } else {
        if (detectionConfig) {

            cv::Mat image = cv::imread(IMAGE_PATH);

            billiard::detection::State pixelState = billiard::snooker::detect(billiard::detection::State{}, image);
            billiard::detection::State state = billiard::detection::pixelToModelCoordinates(*detectionConfig, pixelState);
            printState(state);

            _currentState = map(state);
        }
    }

    _animationChangedEventQueue->push(_currentState);
}

void image() {
    static int imageNumber = 1;
    if (cameraCapture) {
        auto frames = cameraCapture->read();
        cv::imwrite("screenshot-" + std::to_string(imageNumber++) + ".png", frames.color);
    }
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
    KeyFrame keyFrame{0.0, map(state.balls, state.ballSize), state.ballSize, false};

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

Ball* map(const std::vector<billiard::detection::Ball>& ballStates);

std::shared_ptr<RootObject> map(const billiard::detection::State& state) {
    KeyFrame keyFrame{0.0, map(state._balls), static_cast<int>(state._balls.size()), false};

    KeyFrame keyFrames[] = {
            keyFrame,
            keyFrame
    };
    AnimationModel models[] = {AnimationModel{keyFrames, 2}};
    return std::make_shared<RootObject>(models, 1);
}

Ball* map(const std::vector<billiard::detection::Ball>& ballStates) {
    Ball* balls = new Ball[ballStates.size()];

    double scale = 0.001; // millimeters to meters

    for (int i = 0; i < ballStates.size(); i++) {
        auto& ballState = ballStates[i];
        const Vec2& position = Vec2{ballState._position[0] * scale, ballState._position[1] * scale};
        balls[i] = Ball{_strdup(ballState._type.c_str()), _strdup(ballState._id.c_str()), position, Vec2{0, 0}, true};
    }
    return balls;
}

// TODO: Add mapping from internal animation to external animation (Search)

// TODO: Delete all after this comment
std::shared_ptr<RootObject> testState() {
    Ball balls1[] = {
            Ball {
                    "WHITE",
                    "WHITE",
                    Vec2 {-0.4, 0.4},
                    Vec2 {0.172627, -0.172627},
                    true
            },
            Ball {
                    "RED",
                    "RED_1",
                    Vec2 {-0.1, 0.1},
                    Vec2 {0, 0},
                    true
            },
            Ball {
                    "RED",
                    "RED_2",
                    Vec2 {0.1, 0.1},
                    Vec2 {0, 0},
                    true
            }
    };

    Ball balls2[] = {
            Ball {
                    "WHITE",
                    "WHITE",
                    Vec2 {-0.13677, 0.13677},
                    Vec2 {0.090603, -0.090603},
                    true
            },
            Ball {
                    "RED",
                    "RED_1",
                    Vec2 {-0.1, 0.1},
                    Vec2 {0, 0},
                    true
            },
            Ball {
                    "RED",
                    "RED_2",
                    Vec2 {0.1, 0.1},
                    Vec2 {0, 0},
                    false
            }
    };

    Ball balls3[] = {
            Ball {
                    "WHITE",
                    "WHITE",
                    Vec2 {-0.13677, 0.13677},
                    Vec2 {0, 0},
                    true
            },
            Ball {
                    "RED",
                    "RED_1",
                    Vec2 {-0.1, 0.1},
                    Vec2 {0.090603, -0.090603},
                    true
            },
            Ball {
                    "RED",
                    "RED_2",
                    Vec2 {0.1, 0.1},
                    Vec2 {0, 0},
                    false
            }
    };

    Ball balls4[] = {
            Ball {
                    "WHITE",
                    "WHITE",
                    Vec2 {-0.13677, 0.13677},
                    Vec2 {0, 0},
                    true
            },
            Ball {
                    "RED",
                    "RED_1",
                    Vec2 {0.000079, -0.000079},
                    Vec2 {0, 0},
                    false
            },
            Ball {
                    "RED",
                    "RED_2",
                    Vec2 {0.1, 0.1},
                    Vec2 {0, 0},
                    false
            }
    };

    Ball balls5[] = {
            Ball {
                    "WHITE",
                    "WHITE",
                    Vec2 {-0.13677, 0.13677},
                    Vec2 {0, 0},
                    true
            },
            Ball {
                    "RED",
                    "RED_1",
                    Vec2 {0.000079, -0.000079},
                    Vec2 {0, 0},
                    false
            },
            Ball {
                    "RED",
                    "RED_2",
                    Vec2 {0.1, 0.1},
                    Vec2 {0, 0},
                    true
            }
    };

    Ball balls6[] = {
            Ball {
                    "WHITE",
                    "WHITE",
                    Vec2 {-0.13677, 0.13677},
                    Vec2 {0, 0},
                    true
            },
            Ball {
                    "RED",
                    "RED_1",
                    Vec2 {0.000079, -0.000079},
                    Vec2 {0, 0},
                    false
            },
            Ball {
                    "RED",
                    "RED_2",
                    Vec2 {0.1, 0.1},
                    Vec2 {0, 0},
                    false
            }
    };

    KeyFrame keyFrames[6] = {
            KeyFrame {
                0,
                balls1,
                3,
                true
            },
            KeyFrame {
                2,
                balls2,
                3,
                false
            },
            KeyFrame {
                2,
                balls3,
                3,
                false
            },
            KeyFrame {
                4.20918,
                balls4,
                3,
                false
            },
            KeyFrame {
                4.20918,
                balls5,
                3,
                false
            },
            KeyFrame {
                4.3,
                balls6,
                3,
                false
            }
    };

    AnimationModel models[] = {AnimationModel{keyFrames, 6}};
    return std::make_shared<RootObject>(models, 1);
}