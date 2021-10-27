#include <unity_adapter/unity_adapter.hpp>
#include "event_queue.hpp"
#include <memory>
#include <string>
#include <iostream>
#include <fstream>
#include <billiard_detection/billiard_detection.hpp>
#include <billiard_snooker/billiard_snooker.hpp>
#include <billiard_capture/billiard_capture.hpp>
#include <billiard_search/billiard_search.hpp>
#include <billiard_debug/billiard_debug.hpp>
#include <cstring>

// TODO: find a good number
#define SOLUTIONS 10

Debugger _debugger;

bool WRITE_LOG_FILE = true;
bool LIVE = false;
bool STATIC_IMAGE = false;
std::string IMAGE_PATH = "D:\\Dev\\billiard-ai\\cmake-build-debug\\test\\billiard_detection\\resources\\test_detection\\1_scaled_HD.png";

///////////////////////////////////////////////////////
//// Event-Queues
///////////////////////////////////////////////////////
using AnimationChangedEventQueue = unity::EventQueue<RootObject, AnimationChangedEventCallback>;
using StateChangedEventQueue = unity::EventQueue<State, StateChangedEventCallback>;
AnimationChangedEventQueue* _animationChangedEventQueue = nullptr;
StateChangedEventQueue* _stateChangedEventQueue = nullptr;
std::future<std::vector<std::vector<billiard::search::node::System>>> _search;

///////////////////////////////////////////////////////
//// Mappings - Declaration
///////////////////////////////////////////////////////
std::shared_ptr<State> map(const billiard::detection::State& state); // Unity - CPP
std::shared_ptr<RootObject> map(const std::vector<std::vector<billiard::search::node::System>>& simulations); // CPP - Unity
std::shared_ptr<RootObject> mayMap(std::future<std::vector<std::vector<billiard::search::node::System>>>& simulations); // CPP - Unity

///////////////////////////////////////////////////////
//// Implementation
///////////////////////////////////////////////////////
std::shared_ptr<billiard::detection::StateTracker> stateTracker;
std::shared_ptr<billiard::capture::CameraCapture> cameraCapture;
std::shared_ptr<billiard::detection::DetectionConfig> detectionConfig;
std::shared_ptr<billiard::search::Configuration> searchConfig;
std::shared_ptr<State> _currentState;

#ifdef BILLIARD_DEBUG

    std::ofstream _logFileStream {"log.txt"};
    std::ostringstream _unityStream;

    void sendStream(std::ostringstream& stream);
    std::vector<std::string> getMessages(std::string input);
    std::ostream& operator<<(std::ostream& os, const RootObject& root);
#endif

void onStart() {
    _animationChangedEventQueue = new AnimationChangedEventQueue{};
    _stateChangedEventQueue = new StateChangedEventQueue{};

#ifdef BILLIARD_DEBUG
    if (WRITE_LOG_FILE) {
        std::cout.rdbuf(_logFileStream.rdbuf());
    } else {
        std::cout.rdbuf(_unityStream.rdbuf());
    }
#endif
}

void onTearDown() {
    delete _animationChangedEventQueue;
    _animationChangedEventQueue = nullptr;

    delete _stateChangedEventQueue;
    _stateChangedEventQueue = nullptr;
}

void processEvents() {
    auto result = mayMap(_search);
    if (result) {
        DEBUG("Result: " << *result << std::endl);
        DEBUG("Search ended" << std::endl << "---------------------------------------------------------------------------" << std::endl);
        _animationChangedEventQueue->push(result);
    }
    _animationChangedEventQueue->process();

    _stateChangedEventQueue->process();

#ifdef BILLIARD_DEBUG
    if (!WRITE_LOG_FILE) {
        sendStream(_unityStream);
    }
#endif
}

void onAnimationChangedEvent(AnimationChangedEventCallback callback) {
    _animationChangedEventQueue->callback(callback);
}

void onStateChangedEvent(StateChangedEventCallback callback) {
    _stateChangedEventQueue->callback(callback);
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
inline billiard::detection::Table toTable(const Configuration& configuration);
inline billiard::search::Configuration toSearchConfig(const Configuration& config);

void configuration(Configuration config) {
    // TODO: Map configuration and pass it to the library, delete all debugger outputs
    DEBUG("radius: " << std::to_string(config.radius) << " "
    << "width: " << std::to_string(config.width) << " "
       << "height: " << std::to_string(config.height) << " "
       << "segments: " << std::to_string(config.segmentSize) << " "
       << "targets: " << std::to_string(config.targetSize)
       << std::endl);

    DEBUG("marker pattern size: " << std::to_string(config.markers.patternSize) << ", side length: " << std::to_string(config.markers.sideLength) << "mm" << std::endl);
    DEBUG("world to model" << " translation is (" << std::to_string(config.worldToModel.translation.x) << std::to_string(config.worldToModel.translation.y) << ", " << std::to_string(config.worldToModel.translation.z) << ")" << std::endl);
    DEBUG("marker 0" << " is at (" << std::to_string(config.markers.m0.x) << ", " << std::to_string(config.markers.m0.y) << ", " << std::to_string(config.markers.m0.z) << ")" << std::endl);
    DEBUG("marker 1" << " is at (" << std::to_string(config.markers.m1.x) << ", " << std::to_string(config.markers.m1.y) << ", " << std::to_string(config.markers.m1.z) << ")" << std::endl);
    DEBUG("marker 2" << " is at (" << std::to_string(config.markers.m2.x) << ", " << std::to_string(config.markers.m2.y) << ", " << std::to_string(config.markers.m2.z) << ")" << std::endl);
    DEBUG("marker 3" << " is at (" << std::to_string(config.markers.m3.x) << ", " << std::to_string(config.markers.m3.y) << ", " << std::to_string(config.markers.m3.z) << ")" << std::endl);
    DEBUG("ball plane" << " is: (p - " << "(" << std::to_string(config.ballPlane.point.x) << ", " << std::to_string(config.ballPlane.point.y) << ", " << std::to_string(config.ballPlane.point.z) << ")" << ") * " << "(" << std::to_string(config.ballPlane.normal.x) << ", " << std::to_string(config.ballPlane.normal.y) << ", " << std::to_string(config.ballPlane.normal.z) << ")" << " = 0" << std::endl);
    DEBUG("camera intrinsics:"
                << " f: " << "(" << std::to_string(config.camera.fx) << ", " << std::to_string(config.camera.fy) << ")"
                << " c: " << "(" << std::to_string(config.camera.cx) << ", " << std::to_string(config.camera.cy) << ")"
                << " skew: " << std::to_string(config.camera.skew)
                << " k: " << "(" << std::to_string(config.camera.k1) << ", " << std::to_string(config.camera.k2) << ", " << std::to_string(config.camera.k3) << ")"
                << " p: " << "(" << std::to_string(config.camera.p1) << ", " << std::to_string(config.camera.p2) << ")" << std::endl);

    for(int i = 0; i < config.segmentSize; i++) {
        auto& segment = config.segments[i];
        DEBUG("rail segment [" << std::to_string(i) << "] has start at [" << std::to_string(segment.start.x) << ";" << std::to_string(segment.start.y) << "] and end at [" << std::to_string(segment.end.x) << ";" << std::to_string(segment.end.y) << "]" << std::endl);
    }

    for(int i = 0; i < config.targetSize; i++) {
        auto& target = config.targets[i];
        DEBUG("target [" << std::to_string(i) << "] is located at [" << std::to_string(target.position.x) << ";" << std::to_string(target.position.y) << "] and has radius " << std::to_string(target.radius) << std::endl);
    }

    billiard::detection::CameraIntrinsics intrinsics = toIntrinsics(config.camera);
    DEBUG("CameraIntrinsics created");

    billiard::detection::ArucoMarkers markers = createArucoMarkers(config.markers);
    DEBUG("Aruco-board created");
    DEBUG((std::string("Markers:")
               + " patternSize: " + std::to_string(markers.patternSize)
               + " sideLength: " + std::to_string(markers.sideLength)
               + " bottomLeft: " + "" + std::to_string(markers.bottomLeft.x) + ", " + std::to_string(markers.bottomLeft.y) + ", " + std::to_string(markers.bottomLeft.z)
               + " bottomRight: " + "" + std::to_string(markers.bottomRight.x) + ", " + std::to_string(markers.bottomRight.y) + ", " + std::to_string(markers.bottomRight.z)
               + " topLeft: " + "" + std::to_string(markers.topLeft.x) + ", " + std::to_string(markers.topLeft.y) + ", " + std::to_string(markers.topLeft.z)
               + " topRight: " + "" + std::to_string(markers.topRight.x) + ", " + std::to_string(markers.topRight.y) + ", " + std::to_string(markers.topRight.z)
              ).c_str());

    billiard::detection::Table table = toTable(config);
    DEBUG((std::string("Table: ")
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
            DEBUG("[SUCCESS]: Opened camera capture");
        } else {
            DEBUG("[ERROR]: Unable to open camera capture");
            return;
        }

        int skippedFrames = 30;
        while (skippedFrames-- > 0) cameraCapture->read();
        billiard::capture::CameraFrames frames = cameraCapture->read();
        image = frames.color;
    } else if (STATIC_IMAGE) {
        image = cv::imread(IMAGE_PATH);
        cv::imshow("image", image);
    } else {
        // nothing
    }

    if (LIVE || STATIC_IMAGE) {

        detectionConfig = std::make_shared<billiard::detection::DetectionConfig>(billiard::detection::configure(image, table, markers, intrinsics));
        if (detectionConfig->valid) {
            DEBUG("[SUCCESS]: Detection configured");
        } else {
            DEBUG("[ERROR]: Unable to configure detection");
            return;
        }

        if (billiard::snooker::configure(*detectionConfig)) {
            DEBUG("[SUCCESS]: Snooker detection configured");
        } else {
            DEBUG("[ERROR]: Unable to configure snooker detection");
            return;
        }
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

    searchConfig = std::make_shared<billiard::search::Configuration>(toSearchConfig(config));

    DEBUG("All configuration mapped");
}

inline billiard::detection::Table toTable(const Configuration& config) {
    const Table& table = config.table;

    std::vector<billiard::detection::RailSegment> rails {};
    for (int i = 0; i < config.segmentSize; i++) {
        rails.emplace_back(billiard::detection::RailSegment {
                glm::vec2 {config.segments[i].start.x, config.segments[i].start.y},
                glm::vec2 {config.segments[i].end.x, config.segments[i].end.y},
        });
    }

    std::vector<billiard::detection::Pocket> pockets {};
    for(int i = 0; i < config.targetSize; i++) {
        pockets.emplace_back(billiard::detection::Pocket {
                config.targets[i].position.x,
                config.targets[i].position.y,
                config.targets[i].radius
        });
    }

    return billiard::detection::Table {
            table.innerTableLength,
            table.innerTableWidth,
            table.ballDiameter,
            table.arucoHeightAboveInnerTable,
            table.railWorldPointZComponent,
            toVec3d(table.worldToRail),
            pockets,
            rails
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
    DEBUG("[Detection]: Detected " << std::to_string(state._balls.size()) << " balls");
    for (auto& ball : state._balls) {
        DEBUG("[Detection]: Ball: " << ball._id << " " << ball._type << " at (" << std::to_string(ball._position[0]) << ", " << std::to_string(ball._position[1]) << ")");
    }
}

inline billiard::search::RailLocation map(RailLocation location) {
    switch (location) {
        case RailLocation::TOP:
            return billiard::search::RailLocation::TOP;
        case RailLocation::BOTTOM:
            return billiard::search::RailLocation::BOTTOM;
        case RailLocation::LEFT:
            return billiard::search::RailLocation::LEFT;
        case RailLocation::RIGHT:
        default:
            return billiard::search::RailLocation::RIGHT;
    }
}

inline billiard::search::PocketType map(PocketType pocketType) {
    switch (pocketType) {
        case PocketType::CENTER:
            return billiard::search::PocketType::CENTER;
        case PocketType::CORNER:
        default:
            return billiard::search::PocketType::CORNER;
    }
}

inline billiard::search::Configuration toSearchConfig(const Configuration& config) {
    billiard::search::Configuration billiardSearchConfig;

    billiardSearchConfig._ball._radius = config.radius;
    billiardSearchConfig._ball._diameterSquared = (config.radius * 2) * (config.radius * 2);
    billiardSearchConfig._ball._diameter = config.radius * 2;
    billiardSearchConfig._table.minimalPocketVelocity = config.table.minimalPocketVelocity;
    billiardSearchConfig._table.diagonalLengthSquared = config.table.innerTableLength * config.table.innerTableLength
            + config.table.innerTableWidth * config.table.innerTableWidth;

    for (int i = 0; i < config.segmentSize; i++) {
        billiardSearchConfig._table._rails.emplace_back(billiard::search::Rail {
            std::to_string(i),
            glm::vec2{config.segments[i].start.x, config.segments[i].start.y},
            glm::vec2{config.segments[i].end.x, config.segments[i].end.y},
            billiardSearchConfig._ball._radius,
            map(config.segments[i].location)
        });
    }

    for(int i = 0; i < config.targetSize; i++) {
        billiardSearchConfig._table._pockets.emplace_back(billiard::search::Pocket {
           std::string(config.targets[i].id),
           map(config.targets[i].pocketType),
           glm::vec2{config.targets[i].position.x, config.targets[i].position.y},
           glm::vec2{config.targets[i].normal.x, config.targets[i].normal.y},
           config.targets[i].radius
        });
    }

    billiardSearchConfig._table._center = glm::vec2{0, 0};
    billiardSearchConfig._rules._nextSearch = billiard::snooker::nextSearch;
    billiardSearchConfig._rules._modifyState = billiard::snooker::stateAfterBreak;
    billiardSearchConfig._rules._isValidEndState = billiard::snooker::validEndState;
    billiardSearchConfig._rules._scoreForPottedBall = billiard::snooker::scoreForPottedBall;

    return billiardSearchConfig;
}

void capture() {

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

    if (_currentState) {
        _stateChangedEventQueue->push(_currentState);
    }
}

void image() {
    static int imageNumber = 1;
    if (cameraCapture) {
        auto frames = cameraCapture->read();
        cv::imwrite("screenshot-" + std::to_string(imageNumber++) + ".png", frames.color);
        if (!frames.depth.empty()) {
            cv::imwrite("screenshot-" + std::to_string(imageNumber++) + "-depth.png", frames.depth);
        }
        if (!frames.colorizedDepth.empty()) {
            cv::imwrite("screenshot-" + std::to_string(imageNumber++) + "-depth-colorized.png", frames.colorizedDepth);
        }
    }
}

billiard::search::State toSearchState(const std::shared_ptr<State>& state) {
    std::vector<billiard::search::Ball> balls;

    DEBUG("State:" << std::endl);
    for (int i = 0; i < state->ballSize; i++) {
        auto& ball = state->balls[i];
        balls.emplace_back(billiard::search::Ball{
                glm::vec2{ball.position.x, ball.position.y},
                ball.type,
                ball.id,

        });

        DEBUG("" << ball.id << ", " << ball.type << ", " << ball.position.x << ", " << ball.position.y << "" << std::endl);
    }

    return billiard::search::State{ balls };
}

void search(Search search) {
    if (!_currentState) {
        DEBUG("[COMMAND]: search failed -> call capture first" << std::endl);
        return;
    }

    std::stringstream typesString {};

    std::vector<std::string> types;
    for (int i = 0; i < search.typeSize; i++) {
        auto& type = search.types[i];
        types.emplace_back(type.text);
        typesString << type.text << " ";
    }

    DEBUG("Search started, id=" << search.id << " types=[" << typesString.str() << "]"
          << std::endl << "---------------------------------------------------------------------------" << std::endl);

    auto searchInfo = billiard::search::Search{search.id, types};
    _search = billiard::search::search(toSearchState(_currentState), searchInfo, SOLUTIONS, *searchConfig);
}

void debugger(Debugger debugger) {
    _debugger = debugger;
}

void state(State state) {
    _currentState = std::make_shared<State>(state);
    _stateChangedEventQueue->clear();
}

///////////////////////////////////////////////////////
//// Mappings - Implementation
///////////////////////////////////////////////////////

BallState* map(const std::vector<billiard::detection::Ball>& ballStates);

std::shared_ptr<State> map(const billiard::detection::State& state) {
    return std::make_shared<State>(map(state._balls), static_cast<int>(state._balls.size()), false);
}

BallState* map(const std::vector<billiard::detection::Ball>& ballStates) {
    auto* balls = new BallState[ballStates.size()];

    for (int i = 0; i < ballStates.size(); i++) {
        auto& ballState = ballStates[i];
        const Vec2& position = Vec2{ballState._position[0], ballState._position[1]};
        balls[i] = BallState{ballState._type.c_str(), ballState._id.c_str(), position, false};
    }

    return balls;
}

std::shared_ptr<RootObject> mayMap(std::future<std::vector<std::vector<billiard::search::node::System>>>& simulations) {
    if (!simulations.valid() || simulations.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
        return nullptr;
    }
    DEBUG("Solution found!" << std::endl);

    return map(simulations.get());
}

Ball nodeToBall(const std::pair<std::string, billiard::search::node::Node>& node, bool before) {

    auto state = before ? node.second.before() : node.second.after();

    if (state) {
        return Ball {
                node.second._ballType.c_str(),
                node.first.c_str(),
                Vec2 {state->_position.x, state->_position.y},
                Vec2 {state->_velocity.x, state->_velocity.y},
                node.second._type != billiard::search::node::NodeType::BALL_POTTING
        };
    }

    return Ball {
        "UNKNOWN",
        "ERROR",
        Vec2 {0, 0},
        Vec2 {0, 0},
        true
    };
}

std::shared_ptr<RootObject> map(const std::vector<std::vector<billiard::search::node::System>>& simulations) {

    auto* models = new AnimationModel[simulations.size()];

    int modelIndex = 0;
    for (auto& simulation : simulations) {
        std::vector<KeyFrame> keyFrames;
        float biasTime = 0.0f;

        for (auto& system : simulation) {

            for(auto& layer : system._layers) {

                Ball* ballsEnterKeyFrame = new Ball[layer._nodes.size()];
                Ball* ballsExitKeyFrame = new Ball[layer._nodes.size()];

                int ballIndex = 0;
                for(auto& node : layer._nodes) {
                    if (!layer._isFirst) {
                        ballsEnterKeyFrame[ballIndex] = nodeToBall(node, true);
                    }

                    if (!layer._isLast) {
                        ballsExitKeyFrame[ballIndex] = nodeToBall(node, false);
                    }

                    ballIndex++;
                }

                if (!layer._isFirst) {
                    keyFrames.emplace_back(KeyFrame{
                            layer._time + biasTime,
                            ballsEnterKeyFrame,
                            static_cast<int>(layer._nodes.size()),
                            false
                    });
                } else {
                    delete[] ballsEnterKeyFrame;
                }
                if (!layer._isLast) {
                    keyFrames.emplace_back(KeyFrame{
                            layer._time + biasTime,
                            ballsExitKeyFrame,
                            static_cast<int>(layer._nodes.size()),
                            layer._isFirst
                    });
                } else {
                    biasTime += layer._time;
                    delete[] ballsExitKeyFrame;
                }
            }
        }

        auto* keyFramesArray = new KeyFrame[keyFrames.size()];
        std::copy(keyFrames.begin(), keyFrames.end(), keyFramesArray);
        models[modelIndex].keyFrames = keyFramesArray;
        models[modelIndex].keyFrameSize = static_cast<int>(keyFrames.size());
        modelIndex++;
    }

    return std::make_shared<RootObject>(models, simulations.size());
}

#ifdef BILLIARD_DEBUG
    void sendStream(std::ostringstream& stream) {
        if(billiard::debug::Debug::lock()) {
            auto debug = stream.str();
            stream.str("");
            billiard::debug::Debug::unlock();
            if (!debug.empty()) {
                auto messages = getMessages(debug);
                for(auto& message : messages) {
                    _debugger(message.c_str());
                }
            }
        }
        billiard::debug::Debug::unlock();
    }

    std::vector<std::string> getMessages(std::string input) {
        static std::string delimiter = "\n";
        std::vector<std::string> messages;
        size_t pos = 0;
        std::string token;
        while ((pos = input.find(delimiter)) != std::string::npos) {
            token = input.substr(0, pos);
            messages.emplace_back(token);
            input.erase(0, pos + delimiter.length());
        }
        return messages;
    }

    std::ostream& operator<<(std::ostream& os, const Vec2& vec) {
        os << "{" << " "
           << "\"x\":" << vec.x << ", "
           << "\"y\":" << vec.y << ", "
           << " " << "}";
        return os;
    }


    std::ostream& operator<<(std::ostream& os, const Ball& ball) {
        os << "{" << " "
           << "\"id\": \"" << ball.id << "\", "
           << "\"type\": \"" << ball.type << "\", "
           << "\"position\": " << ball.position << ", "
           << "\"velocity\": " << ball.velocity << ", "
           << "\"visible\": " << ball.visible << " "
           << " " << "}";
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const KeyFrame& keyFrame) {
        os << "{" << " "
           << "\"time\": " << keyFrame.time << ", "
           << "\"firstFrame\": " << keyFrame.firstFrame << ", "
           << "\"balls\": [" << " ";
        for (int i = 0; i < keyFrame.ballSize; i++) {
            os << keyFrame.balls[i];
            if (i < keyFrame.ballSize - 1) {
                os << ", ";
            }
        }
        os << " " << "]";
        os << " " << "}";
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const AnimationModel& animation) {
        os << "{" << " "
           << "\"keyFrameSize\": " << animation.keyFrameSize << ", "
           << "\"keyframes\": [ ";

        for(int i = 0; i < animation.keyFrameSize; i++) {
            os << animation.keyFrames[i];
            if (i < animation.keyFrameSize - 1) {
                os << ", ";
            }
        }
        os << " " << "]";
        os << " " << "}";
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const RootObject& root) {
        os << "{" << " "
           << "\"animationSize\": " << root.animationSize << ", "
           << "\"animations\": [";
        for(int i = 0; i < root.animationSize; i++) {
            os << root.animations[i];
            if (i < root.animationSize - 1) {
                os << ", ";
            }
        }
        os << " " << "]";
        os << " " << "}";
        return os;
    }
#endif