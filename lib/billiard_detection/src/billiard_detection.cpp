#include <billiard_detection/billiard_detection.hpp>
#include <billiard_debug/billiard_debug.hpp>

#ifndef BILLIARD_DETECTION_DEBUG_VISUAL
    #ifdef NDEBUG
        #undef BILLIARD_DETECTION_DEBUG_VISUAL
    #endif
    #ifndef NDEBUG
        #define BILLIARD_DETECTION_DEBUG_VISUAL 1
    #endif
#endif
#ifndef BILLIARD_DETECTION_DEBUG_PRINT
#ifdef NDEBUG
        #undef BILLIARD_DETECTION_DEBUG_PRINT
    #endif
    #ifndef NDEBUG
        #define BILLIARD_DETECTION_DEBUG_PRINT 1
    #endif
#endif

// TODO: remove this
//#define BILLIARD_DETECTION_DEBUG_VISUAL 1
#undef BILLIARD_DETECTION_DEBUG_VISUAL
//#define BILLIARD_DETECTION_DEBUG_PRINT 1
#undef BILLIARD_DETECTION_DEBUG_PRINT
//#define BILLIARD_TRACKING_DEBUG_PRINT 1
#undef BILLIARD_TRACKING_DEBUG_PRINT

#ifdef BILLIARD_DETECTION_DEBUG_PRINT
#define DETECTION_DEBUG(x) DEBUG(x)
#else
#define DETECTION_DEBUG(x) do {} while (0)
#endif

#ifdef BILLIARD_TRACKING_DEBUG_PRINT
#define TRACKING_DEBUG(x) DEBUG(x)
#else
#define TRACKING_DEBUG(x) do {} while (0)
#endif

billiard::detection::StateTracker::StateTracker(const std::function<capture::CameraFrames ()>& capture,
                                                const std::shared_ptr<billiard::detection::DetectionConfig>& config,
                                                const std::function<State(const State& previousState,
                                                                          const cv::Mat&)>& detect,
                                                const std::function<void (const State& previousState,
                                                                          State& currentState,
                                                                          const cv::Mat&)>& classify)
        :
        _exitSignal(),
        _lock(),
        _waiting(),
        _thread(std::thread(StateTracker::work, _exitSignal.get_future(), std::ref(_lock),
                            capture, config, detect, classify, std::ref(_waiting))) {
}

billiard::detection::StateTracker::~StateTracker() {
    _exitSignal.set_value();
    _thread.join();
}

std::future<billiard::detection::State> billiard::detection::StateTracker::capture() {
    std::promise<State> promise;
    auto future = promise.get_future();
    _lock.lock();
    _waiting.emplace(std::move(promise));
    _lock.unlock();
    return future;
}

struct TrackedBall {
    bool tracked;
    float distance;
    int previousStateIndex;
    int currentStateIndex;
    TrackedBall(bool tracked, float distance, int previousStateIndex, int currentStateIndex):
            tracked(tracked),
            distance(distance),
            previousStateIndex(previousStateIndex),
            currentStateIndex(currentStateIndex) {
    }
};

template<int n, typename T>
void addValue(billiard::detection::CircularBuffer<n, T>& buffer, const T& value) {

    buffer.values[buffer.index++] = value;
    if (buffer.index == n) {
        // Buffer full, overwrite oldest entries
        buffer.index = 0;
    }
    if (buffer.count < n) {
        // Buffer not full yet
        buffer.count++;
    }
}

template<int n, typename T>
glm::vec2 calculateAverage(const billiard::detection::CircularBuffer<n, T>& buffer, T total) {
    for (int i = 0 ; i < buffer.count; i++) {
        total += buffer.values[i];
    }
    return total / ((float) buffer.count);
}

glm::vec2 calculateAveragePosition(const billiard::detection::BallStats& stats) {
    return calculateAverage(stats.positions, glm::vec2{0, 0});
}

glm::vec2 calculateAverageMovement(const billiard::detection::BallStats& stats) {
    return calculateAverage(stats.movement, glm::vec2{0, 0});
}

int findNearestBall(const glm::vec2& position,
                    const bool* skip,
                    const billiard::detection::State& state,
                    float maxTrackingDistanceSquared) {

    float minDistanceSquared = 2.0f * maxTrackingDistanceSquared; // just some number bigger than any distance considered
    int bestMatchIndex = -1;

    for (int index = 0; index < state._balls.size(); index++) {
        if (skip[index]) {
            continue;
        }

        auto& ball = state._balls[index];
        glm::vec2 distance = position - ball._position;
        float distanceSquared = glm::dot(distance, distance);

        if (distanceSquared < maxTrackingDistanceSquared) {
            if (distanceSquared < minDistanceSquared) {
                minDistanceSquared = distanceSquared;
                bestMatchIndex = index;
            }
        }
    }

    return bestMatchIndex;
}

float calculatePositionMixingFactor(const glm::vec2& currentPosition,
                                    const glm::vec2& averagePosition,
                                    float maxDistanceSquared) {

    glm::vec2 delta = averagePosition - currentPosition;
    float distanceSquared = glm::dot(delta, delta);
    // Factor is 1.0 if distance is high -> use current position
    // Factor is 0.0 if distance is low -> use average position
    return std::min(distanceSquared / maxDistanceSquared, 1.0f);
}

glm::vec2 calculateNewPosition(const glm::vec2& currentPosition,
                               const glm::vec2& averagePosition,
                               float maxDistanceSquared) {

    float factor = calculatePositionMixingFactor(currentPosition, averagePosition, maxDistanceSquared);

    // Mix current position and average position based on factor
    return factor * currentPosition + (1.0f - factor) * averagePosition;
}

std::string readable(const billiard::detection::TableStatus& status) {
    using billiard::detection::TableStatus;
    switch (status) {
        case TableStatus::UNKNOWN:
            return "UNKNOWN";
        case TableStatus::STABLE:
            return "STABLE";
        case TableStatus::UNSTABLE:
            return "UNSTABLE";
    }

    return "No TableStatus found";
}

std::string readable(const billiard::detection::CueBallStatus& status) {
    using billiard::detection::CueBallStatus;
    switch (status) {
        case CueBallStatus::UNKNOWN:
            return "UNKNOWN";
        case CueBallStatus::FOUND:
            return "FOUND";
        case CueBallStatus::LOST:
            return "LOST";
    }

    return "No CueBallStatus found";
}

std::ostream& operator<<(std::ostream& os, const std::vector<std::string>& values) {
    os << "[ ";
    for (auto& value : values) {
        os << value << " ";
    }
    os << "]";
    return os;
}

void trackCueBall(billiard::detection::Tracking& tracking,
                  const billiard::detection::State& previousState,
                  const billiard::detection::State& currentState,
                  float maxCueBallTrackingSquaredDistance,
                  float maxStableAverageMovementSquared) {

    std::string agent = "[trackCueBall] ";
    std::stringstream debugOutput;

    int previousCueBallIndex = tracking.cueBallIndex;
    int cueBallIndex = -1;
    int whiteBallCount = 0;
    auto oldStatus = tracking.cueBallStatus;
    auto newStatus = billiard::detection::CueBallStatus::UNKNOWN;

    bool* skip = new bool[currentState._balls.size()];

    for (int i = 0; i < currentState._balls.size(); i++) {
        auto& ball = currentState._balls[i];

        if (ball._type == "WHITE") {

            whiteBallCount++;

            if (cueBallIndex == -1) {

                // One cue ball found
                cueBallIndex = i;
                newStatus = billiard::detection::CueBallStatus::FOUND;
                debugOutput << "At least one white ball found." << " ";

            } else if (previousCueBallIndex >= 0) {
                // Multiple white balls found, try finding it around the previous position

                for (int j = 0; j < currentState._balls.size(); j++) {
                    skip[j] = currentState._balls[j]._type != "WHITE";
                }

                auto& previousCueBallPosition = previousState._balls[previousCueBallIndex]._position;
                cueBallIndex = findNearestBall(previousCueBallPosition, skip, currentState, maxCueBallTrackingSquaredDistance);

                if (cueBallIndex >= 0) {
                    // Cue ball found
                    newStatus = billiard::detection::CueBallStatus::FOUND;
                    debugOutput << "Multiple white balls, found one using previous position." << " ";
                } else {
                    // Cue ball could not be found around previous position
                    newStatus = billiard::detection::CueBallStatus::LOST;
                    debugOutput << "Multiple white balls, but could not decide based on previous position." << " ";
                }
                break;

            } else {
                // Multiple white balls found, but no previous position known
                cueBallIndex = -1;
                newStatus = billiard::detection::CueBallStatus::LOST;
                debugOutput << "Multiple white balls, no previous position known." << " ";
                break;
            }
        }
    }

    if (whiteBallCount == 0) {
        // If no white ball was found do not change status:
        // State probably is UNSTABLE because in order for the cueball to go missing, there had to be movement of any kind.
        // Once the cue ball is put back or is detected again, the state will begin to stabilize.
        newStatus = oldStatus;
        debugOutput << "No white ball found." << " ";
    }

    tracking.previousCueBallIndex = previousCueBallIndex;
    tracking.cueBallIndex = cueBallIndex;
    tracking.cueBallStatus = newStatus;

    TRACKING_DEBUG(agent
                  << "cue ball: " << cueBallIndex  << " "
                  << "status: " << readable(newStatus) << " "
                  << debugOutput.str()
                  << std::endl);
}

int countLostBalls(const billiard::detection::State& state, const bool tracked[], int minTrackedDurationBeforeLost) {

    int lostCount = 0;
    for (int i = 0; i < state._balls.size(); i++) {
        if (!tracked[i]) {
            // Ball may have been lost
            auto& ball = state._balls[i];
            if (ball._trackingCount > minTrackedDurationBeforeLost) {
                // Ball was tracked long enough to be considered a lost ball
                // (Balls not tracked long enough may just be detection errors)
                lostCount++;
            }
        }
    }
    return lostCount;
}

billiard::detection::State track(billiard::detection::Tracking& tracking,
                                 const billiard::detection::State& previousState,
                                 const billiard::detection::State& currentState,
                                 float maxTrackingDistanceSquared,
                                 int minTrackingCountBeforeStable,
                                 int minTrackedDurationBeforeLost,
                                 int minTrackedDurationBeforeMoving,
                                 float maxStableAverageMovementSquared) {

    std::string agent = "[track] ";

    int previousStableTrackings = tracking.stableTrackings;
    tracking.reset();

    billiard::detection::State result;
    result._balls.reserve(currentState._balls.size());

    std::vector<TrackedBall> trackedBalls;
    trackedBalls.reserve(currentState._balls.size());

    std::vector<std::string> alreadyAssignedIds;
    alreadyAssignedIds.reserve(currentState._balls.size());

    int previousBallCount = previousState._balls.size();
    bool* tracked = new bool[previousBallCount];
    for (int previousBallIndex = 0; previousBallIndex < previousBallCount; previousBallIndex++) {
        tracked[previousBallIndex] = false;
    }
    std::list<int> untrackedBallIndices;
    std::set<std::string> trackedIds;

    trackCueBall(tracking, previousState, currentState, maxTrackingDistanceSquared, maxStableAverageMovementSquared);

    for (int currentBallIndex = 0; currentBallIndex < currentState._balls.size(); currentBallIndex++) {

        auto& currentBall = currentState._balls[currentBallIndex];
        glm::vec2 currentPosition = currentBall._position;

        int bestMatchIndex = -1;
        if (currentBallIndex == tracking.cueBallIndex) {
            bestMatchIndex = tracking.previousCueBallIndex;
        } else {
            bestMatchIndex = findNearestBall(currentPosition, tracked, previousState, maxTrackingDistanceSquared);
        }

        if (bestMatchIndex >= 0) {
            // Successful tracking
            tracked[bestMatchIndex] = true;
            int previousBallIndex = bestMatchIndex;

            auto& previousBall = previousState._balls[previousBallIndex];
            auto& id = previousBall._id;
            alreadyAssignedIds.push_back(id);
            trackedIds.insert(id);

            auto& detectedPosition = currentBall._position;

            billiard::detection::BallStats& stats = tracking.stats[id];
            addValue(stats.positions, detectedPosition);

            glm::vec2 newPosition;

            std::stringstream debugOutput;

            if (stats.positions.count >= WARMED_UP_LIMIT) {
                // History is warmed up, use average position from history
                glm::vec2 averagePosition = calculateAveragePosition(stats);
                newPosition = calculateNewPosition(currentPosition, averagePosition, maxTrackingDistanceSquared);

                float mixingFactor = calculatePositionMixingFactor(currentPosition, averagePosition, maxTrackingDistanceSquared);
                debugOutput
                      << "Successful " << id << ", "
                      << "avg: (" << averagePosition.x << ", " << averagePosition.y << ") "
                      << "current: (" << currentBall._position.x << ", " << currentBall._position.y << ") "
                      << "factor: " << mixingFactor << " "
                      << "new: (" << newPosition.x << ", " << newPosition.y << ") "
                      << "moved: " << glm::length(previousState._balls[previousBallIndex]._position - currentBall._position) << " ";
            } else {
                // History is not warmed up yet, use detected position
                newPosition = detectedPosition;

                debugOutput
                      << "Successful " << id << ", " << "but history not warmed-up yet" << ", "
                      << "new: (" << newPosition.x << ", " << newPosition.y << ") "
                      << "moved: " << glm::length(previousState._balls[previousBallIndex]._position - currentBall._position);
            }

            glm::vec2 movement = newPosition - previousBall._position;
            addValue(stats.movement, movement);

            int trackingCount = previousBall._trackingCount + 1;

            tracking.trackedCount++;
            if (trackingCount >= minTrackingCountBeforeStable) {
                tracking.stableTrackings++;
            } else {
                tracking.unstableTrackings++;
            }

            if (stats.movement.count >= WARMED_UP_LIMIT) {
                // History is warmed up, use average movement from history
                glm::vec2 averageMovement = calculateAverageMovement(stats);

                if (glm::dot(averageMovement, averageMovement) >= 0.1f * 0.1f) {
                    debugOutput << "avg movement: (" << averageMovement.x << ", " << averageMovement.y << ") ";
                }

                bool moving = glm::dot(averageMovement, averageMovement) >= maxStableAverageMovementSquared;
                if (trackingCount > minTrackedDurationBeforeMoving && moving) {
                    // Moved too much, the ball must be moving
                    tracking.movingCount++;
                    tracking.moving.push_back(id);
                }
            }

            billiard::detection::Ball ball;
            ball._id = id;
            ball._type = currentBall._type;
            ball._position = newPosition;
            ball._trackingCount = trackingCount;
            result._balls.push_back(ball);

            TRACKING_DEBUG(agent << debugOutput.str() << std::endl);

        } else {
            // Could not track ball
            untrackedBallIndices.push_back(currentBallIndex);
            tracking.untrackedCount++;
        }
    }

    tracking.lostCount = countLostBalls(previousState, tracked, minTrackedDurationBeforeLost);

    int stableTrackingsChange = tracking.stableTrackings - previousStableTrackings;
    // Consider positive values only, because lost balls are already handled with a different metric
    tracking.addedCount = std::max(stableTrackingsChange, 0);

    // Cleanup tracking history of lost balls
    for (auto& t : tracking.tracked) {
        if (std::find(trackedIds.begin(), trackedIds.end(), t) == trackedIds.end()) {
            tracking.stats.erase(t);
        }
    }
    tracking.tracked = trackedIds;

    // Assign IDs to untracked balls that do not collide with tracked balls
    int number = 0;
    for (auto ballIndex : untrackedBallIndices) {
        auto& currentBall = currentState._balls[ballIndex];

        std::string id = currentBall._type + "-" + std::to_string(number++);
        while (std::find(alreadyAssignedIds.begin(), alreadyAssignedIds.end(), id) != alreadyAssignedIds.end()) {
            id = currentBall._type + "-" + std::to_string(number++);
        }

        billiard::detection::Ball ball;
        ball._id = id;
        ball._type = currentBall._type;
        ball._position = currentBall._position;
        ball._trackingCount = 0;
        result._balls.push_back(ball);

        TRACKING_DEBUG(agent
              << "Failed " << id << " "
              << "new: (" << currentBall._position.x << ", " << currentBall._position.y << ") "
              << std::endl);
    }

    TRACKING_DEBUG(agent
          << "Tracked: " << tracking.tracked.size() << " "
          << "[ ");
    for (auto& tracked : tracking.tracked) {
        TRACKING_DEBUG(tracked << ", ");
    }
    TRACKING_DEBUG("]" << std::endl);

    delete[] tracked;
    return result;
}

billiard::detection::TableStatus determineTableStatus(const billiard::detection::Tracking& tracking,
                                                      const billiard::detection::TableStatus& currentStatus,
                                                      const billiard::detection::State& previousState,
                                                      const billiard::detection::State& trackedState) {

    using billiard::detection::CueBallStatus;
    using billiard::detection::TableStatus;

    std::string agent = "[determineTableStatus] ";

    if (tracking.cueBallStatus == CueBallStatus::UNKNOWN) {
        return TableStatus::UNKNOWN;
    }

    int currentBallCount = trackedState._balls.size();
    bool cueBallFound = tracking.cueBallStatus == CueBallStatus::FOUND;

    if (cueBallFound && tracking.movingCount == 0 && tracking.lostCount == 0 && tracking.addedCount == 0) {

        if (currentBallCount == tracking.stableTrackings) {
            TRACKING_DEBUG(agent << "STABLE" << " "
                       << "Cue ball: " << readable(tracking.cueBallStatus) << " "
                       << "tracked: " << std::to_string(tracking.trackedCount) << " untracked: " << std::to_string(tracking.untrackedCount) << " "
                       << "stable trackings: " << std::to_string(tracking.stableTrackings) << " "
                       << "added: " << std::to_string(tracking.addedCount) << " "
                       << "lost: " << std::to_string(tracking.lostCount) << " "
                       << "moving: " << std::to_string(tracking.movingCount) << " "
                       << "" << tracking.moving << " "
                       << std::endl);
            return TableStatus::STABLE;
        } else {
            // Do not change status because not all balls have stabilized,
            // that may be because of ghost balls appearing in a stable state, in which case we do not want to change to UNSTABLE,
            // or it may be because some ball is moving and could not be tracked and is therefore classified as a ghost in an unstable state, in which case we do not want to change to STABLE.
            TRACKING_DEBUG(agent  << "INDECISIVE" << " "
                       << "Cue ball: " << readable(tracking.cueBallStatus) << " "
                       << "tracked: " << std::to_string(tracking.trackedCount) << " untracked: " << std::to_string(tracking.untrackedCount) << " "
                       << "stable trackings: " << std::to_string(tracking.stableTrackings) << " "
                       << "added: " << std::to_string(tracking.addedCount) << " "
                       << "lost: " << std::to_string(tracking.lostCount) << " "
                       << "moving: " << std::to_string(tracking.movingCount) << " "
                       << "" << tracking.moving << " "
                       << std::endl);
            return currentStatus;
        }
    } else {
        TRACKING_DEBUG(agent << "UNSTABLE" << " "
                   << "Cue ball: " << readable(tracking.cueBallStatus) << " "
                   << "tracked: " << std::to_string(tracking.trackedCount) << " untracked: " << std::to_string(tracking.untrackedCount) << " "
                   << "stable trackings: " << std::to_string(tracking.stableTrackings) << " "
                   << "added: " << std::to_string(tracking.addedCount) << " "
                   << "lost: " << std::to_string(tracking.lostCount) << " "
                   << "moving: " << std::to_string(tracking.movingCount) << " "
                   << "" << tracking.moving << " "
                   << std::endl);
        return TableStatus::UNSTABLE;
    }
}

billiard::detection::TableStatus trackTableState(billiard::detection::Tracking& tracking,
                                                 const billiard::detection::TableStatus& currentStatus,
                                                 const billiard::detection::State& previousState,
                                                 const billiard::detection::State& trackedState,
                                                 int minStableStateCount) {

    using billiard::detection::TableStatus;

    TableStatus status = determineTableStatus(tracking, currentStatus, previousState, trackedState);

    if (status == TableStatus::STABLE) {
        tracking.stableStateCount++;
        if (tracking.stableStateCount >= minStableStateCount) {
            // Pretty sure that state is STABLE now
            return TableStatus::STABLE;
        } else {
            // Not sure yet whether state is really STABLE
            return currentStatus;
        }
    } else {
        tracking.stableStateCount = 0;
        return status;
    }
}

const float MAX_TRACKING_DISTANCE_SQUARED = 20.f * 20.0f;
const float MAX_BALL_STABLE_AVERAGE_MOVEMENT_SQUARED = 0.5f * 0.5f;

// Minimal duration that a ball has to be tracked successfully before
// being counted as a ball in the total of balls present on the table
const int MIN_TRACKING_COUNT_BEFORE_STABLE = 30;

// Minimal duration that a ball has to be tracked successfully before
// being considered lost, when tracking fails
const int MIN_TRACKING_COUNT_BEFORE_LOST = 30;

// Minimal duration that a ball has to be tracked successfully before
// being considered moving, when motion is detected
const int MIN_TRACKING_COUNT_BEFORE_MOVING = 30;

// Minimal number of times the state has to be classified as STABLE consecutively before changing state to STABLE
const int MIN_STABLE_STATE_COUNT = 10;

void billiard::detection::StateTracker::work(std::future<void> exitSignal,
          std::mutex& lock,
          const std::function<capture::CameraFrames ()>& capture,
          const std::shared_ptr<billiard::detection::DetectionConfig>& config,
          const std::function<State (const State& previousState, const cv::Mat&)>& detect,
          const std::function<void (const State& previousState, State& currentState, const cv::Mat&)>& classify,
          std::queue<std::promise<State>>& waiting) {
    Tracking tracking;
    TableStatus previousStatus = TableStatus::UNKNOWN;
    State previousPixelState;
    State previousModelState;
    std::string currentDisplayText = "";
    std::string previousDisplayText = "";
    std::chrono::system_clock::time_point showSearchingUntil;

    while (exitSignal.wait_for(std::chrono::nanoseconds(10)) == std::future_status::timeout) {
        auto image = capture();
        if (image.color.empty()) {
            continue;
        }
        auto pixelState = detect(previousPixelState, image.color);
        classify(previousPixelState, pixelState, image.color);

        // Convert state to internal coordinates
        auto modelState = pixelToModelCoordinates(*config, pixelState);
        // Track balls and smoothen the detected positions over time
        auto trackedState = track(tracking, previousModelState, modelState, MAX_TRACKING_DISTANCE_SQUARED, MIN_TRACKING_COUNT_BEFORE_STABLE, MIN_TRACKING_COUNT_BEFORE_LOST, MIN_TRACKING_COUNT_BEFORE_MOVING, MAX_BALL_STABLE_AVERAGE_MOVEMENT_SQUARED);
        TableStatus status = trackTableState(tracking, previousStatus, previousModelState, trackedState, MIN_STABLE_STATE_COUNT);
        trackedState.status = status;

#if BILLIARD_DETECTION_DEBUG_VISUAL
//#if 1
        std::vector<cv::Point2d> modelPoints;
        for (billiard::detection::Ball& ball : trackedState._balls) {
            modelPoints.emplace_back(ball._position.x, ball._position.y);
        }
        auto worldPoints = billiard::detection::modelPointsToWorldPoints(config->worldToModel, modelPoints, 0.0);
        auto imagePoints = billiard::detection::worldPointsToImagePoints(config->cameraToWorld, worldPoints);

        cv::Mat trackedResult;
        image.color.copyTo(trackedResult);
        for (auto& point : imagePoints) {
            cv::Point center = cv::Point(point.x, point.y);
            cv::circle(trackedResult, center, 1, cv::Scalar(0, 100, 100), 3, cv::LINE_AA);
            cv::circle(trackedResult, center, config->ballRadiusInPixel, cv::Scalar(255, 0, 255), 1, cv::LINE_AA);
        }

        if (status != previousStatus) {
            previousDisplayText = currentDisplayText;

            if (status == TableStatus::STABLE) {
                TRACKING_DEBUG("[work] " << "State stabilized, search" << std::endl);
                showSearchingUntil = std::chrono::system_clock::now() + std::chrono::milliseconds(500);
            }
        }


        std::string displayText = readable(status)
                + " - "
                + "Cue: " + readable(tracking.cueBallStatus) + " "
                + "lost: " + std::to_string(tracking.lostCount) + " "
                + "moving: " + std::to_string(tracking.movingCount) + " "
                + "added: " + std::to_string(tracking.addedCount) + " ";
        currentDisplayText = displayText;

        cv::rectangle(trackedResult, cv::Rect {0, trackedResult.rows - 75, (int)(trackedResult.cols * 0.5), 75}, cv::Scalar{255, 255, 255}, cv::FILLED);
        cv::putText(trackedResult, displayText, cv::Point {25, trackedResult.rows - 25}, cv::FONT_HERSHEY_PLAIN, 1.0, cv::Scalar {255, 0, 0});
        cv::putText(trackedResult, previousDisplayText, cv::Point {25, trackedResult.rows - 50}, cv::FONT_HERSHEY_PLAIN, 1.0, cv::Scalar {255, 0, 0});
        if (showSearchingUntil > std::chrono::system_clock::now()) {
            cv::putText(trackedResult, "Searching", cv::Point {trackedResult.cols/2 - 100, trackedResult.rows/2}, cv::FONT_HERSHEY_PLAIN, 4.0, cv::Scalar {255, 0, 0});
        }
        cv::imshow("Tracked", trackedResult);
        cv::waitKey(1);
#endif

        previousPixelState = pixelState;
        previousModelState = trackedState;
        previousStatus = status;

        lock.lock();
        if (!waiting.empty()) {
            while (!waiting.empty()) {
                auto& prom = waiting.front();
                prom.set_value(trackedState);
                waiting.pop();
            }
        }
        lock.unlock();
    }
}

namespace billiard::detection {

    DetectedMarkers detect(const cv::Mat& image, const cv::Ptr<cv::aruco::Board>& board) {
        DetectedMarkers result{};

        cv::Ptr<cv::aruco::DetectorParameters> parameters = cv::aruco::DetectorParameters::create();
        cv::aruco::detectMarkers(image, board->dictionary, result.markerCorners,
                                 result.markerIds, parameters, result.rejectedCandidates);

        result.success = result.markerIds.size() > 0;
        return result;
    }

    void draw(cv::Mat& image, const DetectedMarkers& detection) {
        cv::aruco::drawDetectedMarkers(image, detection.markerCorners, detection.markerIds);
    }

    /**
     * Estimates the pose of a board. According to the OpenCV documentation:
     * "The returned transformation is the one that transforms points from
     * the board coordinate system to the camera coordinate system."
     */
    BoardPoseEstimation estimateBoardPose(const DetectedMarkers& detectionResult,
                                          const cv::Ptr<cv::aruco::Board>& board,
                                          const CameraIntrinsics& intrinsics) {

        BoardPoseEstimation pose;
        pose.markersUsed = cv::aruco::estimatePoseBoard(detectionResult.markerCorners,
                                                        detectionResult.markerIds, board,
                                                        intrinsics.cameraMatrix, intrinsics.distCoeffs,
                                                        pose.rvec, pose.tvec);
        return pose;
    }

    void drawAxis(cv::Mat& image, const BoardPoseEstimation& pose, const CameraIntrinsics& intrinsics) {
        cv::aruco::drawAxis(image, intrinsics.cameraMatrix, intrinsics.distCoeffs, pose.rvec, pose.tvec, 100);
    }

    cv::Mat buildBoardToCameraMatrix(const BoardPoseEstimation& pose) {

        cv::Mat rotationMatrix;
        cv::Rodrigues(pose.rvec, rotationMatrix);

        cv::Mat boardToCamera{cv::Size{4, 4}, CV_64F, cv::Scalar(0.0f)};
        for (int row = 0; row <= 2; row++) {
            for (int col = 0; col <= 2; col++) {
                boardToCamera.at<double>(row, col) = rotationMatrix.at<double>(row, col);
            }
        }
        for (int row = 0; row <= 2; row++) {
            boardToCamera.at<double>(row, 3) = pose.tvec[row];
        }
        boardToCamera.at<double>(3, 3) = 1.0;

        return boardToCamera;
    }

    CameraToWorldCoordinateSystemConfig configure(const cv::Mat& image,
                                                  const cv::Ptr<cv::aruco::Board>& board,
                                                  const CameraIntrinsics& intrinsics) {

        DetectedMarkers detection = detect(image, board);

        if (detection.success) {

            BoardPoseEstimation pose = estimateBoardPose(detection, board, intrinsics);
            if (pose.markersUsed > 0) {

                CameraToWorldCoordinateSystemConfig config;
                config.valid = true;
                config.pose = pose;
                config.intrinsics = intrinsics;
                config.boardToCamera = buildBoardToCameraMatrix(pose);
                cv::invert(config.boardToCamera, config.cameraToBoard);

                cv::Vec4d cameraInCameraCoordinates{0, 0, 0, 1};
                cv::Mat cameraInWorldCoordinatesHomogeneous = config.cameraToBoard * cameraInCameraCoordinates;
                cv::Mat cameraInWorldCoordinatesMat = cameraInWorldCoordinatesHomogeneous(cv::Rect{0, 0, 1, 3});
                config.cameraPosInWorldCoordinates = cv::Vec3d(cameraInWorldCoordinatesMat);

                config.principalPoint = {intrinsics.cameraMatrix.at<double>(0, 2),
                                         intrinsics.cameraMatrix.at<double>(1, 2)};
                double fx = intrinsics.cameraMatrix.at<double>(0, 0);
                double fy = intrinsics.cameraMatrix.at<double>(1, 1);
                config.focalLength = fx * intrinsics.sensorSize.x;

                DETECTION_DEBUG("camerToWorld:configure"
                      << "cameraInWorldCoordinatesHomogeneous: " << cameraInWorldCoordinatesHomogeneous
                      << std::endl
                      << "cameraInWorldCoordinatesMat: " << cameraInWorldCoordinatesMat
                      << std::endl
                      << "Camera is at " << config.cameraPosInWorldCoordinates << " in the real world"
                      << std::endl
                      << "f: " << std::to_string(config.focalLength)
                      << " fx: " << std::to_string(fx)
                      << " fy: " << std::to_string(fy)
                      << " s: " << intrinsics.sensorSize
                      << " c: " << config.principalPoint
                      << std::endl);

                return config;
            }
        }
        return CameraToWorldCoordinateSystemConfig{};
    }

    std::vector<cv::Point2d> worldPointsToImagePoints(const CameraToWorldCoordinateSystemConfig& config,
                                                      const std::vector<cv::Point3d>& worldPoints) {

        std::vector<cv::Point2d> imagePoints;
        cv::projectPoints(worldPoints, config.pose.rvec, config.pose.tvec,
                          config.intrinsics.cameraMatrix, config.intrinsics.distCoeffs,
                          imagePoints);
        return imagePoints;
    }

    cv::Point3d linePlaneIntersection(const cv::Vec3d& linePoint, const cv::Vec3d& lineDirection,
                                      const cv::Vec3d& planePoint, const cv::Vec3d& planeNormal) {

        // Given a plane as (p - a) * n = 0
        // where
        // - n is the normal vector of the plane
        // - a is a known point on the plane
        // - p is a variable point on the plane
        //
        // And given a line as p = q + lambda * v
        // where
        // - v is the direction vector of the line
        // - q is a known point on the line
        // - p is a variable point on the line
        // - lambda is the scaling factor of the direction vector v
        //
        // Then the intersection can be found by inserting the line equation into the equation of the plane
        // (q + lambda * v - a) * n = 0
        // q * n + lambda * v * n - a * n = 0
        // lambda * v * n = a * n - q * n
        // lambda * v * n = (a - q) * n
        // lambda = ((a - q) * n) / (v * n)
        //
        // Note that if v * n = 0, then there is no intersection
        //
        double vDotP = lineDirection.dot(planeNormal);
        assert(vDotP != 0.0);

        double lambda = (planePoint - linePoint).dot(planeNormal) / vDotP;

        cv::Point3d worldPoint(linePoint + (lambda * lineDirection));

        DETECTION_DEBUG("[linePlaneIntersection] "
                  << "Line: " << linePoint << " + " << std::to_string(lambda) << " * " << lineDirection
                  << std::endl
                  << "Plane: (p - " << planePoint << ") * " << planeNormal << " = 0"
                  << std::endl);

        return worldPoint;
    }

    std::vector<cv::Point3d> imagePointsToWorldPoints(const CameraToWorldCoordinateSystemConfig& config,
                                                      const Plane& plane,
                                                      const std::vector<cv::Point2d>& imagePoints) {

        std::vector<cv::Point3d> worldPoints;

        cv::Point2d s = config.intrinsics.sensorSize;
        cv::Point2d c = config.principalPoint;
        double f = config.focalLength;

        for (auto& imagePoint : imagePoints) {

            cv::Vec4d imagePointInCameraCoordinatesHomogenous{(imagePoint.x - c.x) * s.x,
                                                              (imagePoint.y - c.y) * s.y,
                                                              f,
                                                              1};

            cv::Mat imagePointInWorldCoordinatesMat = config.cameraToBoard * imagePointInCameraCoordinatesHomogenous;
            cv::Vec3d imagePointInWorldCoordinates = cv::Vec3d(
                    imagePointInWorldCoordinatesMat.rowRange(cv::Range(0, 3)));

            cv::Vec3d linePoint = config.cameraPosInWorldCoordinates;
            cv::Vec3d lineDirection = imagePointInWorldCoordinates - config.cameraPosInWorldCoordinates;

            cv::Point3d worldPoint = linePlaneIntersection(linePoint, lineDirection, plane.point, plane.normal);

            DETECTION_DEBUG("[imagePointsToWorldPoints] "
                  << "image: " << imagePoint << " "
                  << "camera: " << imagePointInCameraCoordinatesHomogenous << " "
                  << "world: " << imagePointInWorldCoordinates << " "
                  << std::endl);

            worldPoints.push_back(worldPoint);
        }

        return worldPoints;
    }

    std::vector<cv::Point2d> worldPointsToModelPoints(
            const WorldToModelCoordinates& worldToModel,
            const std::vector<cv::Point3d>& worldPoints) {

        const cv::Point3d& translation = cv::Point3d(worldToModel.translation);
        std::vector<cv::Point2d> modelPoints;
        for (const cv::Point3d& worldPoint : worldPoints) {
            cv::Point3d modelPoint = worldPoint + translation;
            modelPoints.emplace_back(modelPoint.x, modelPoint.y);
        }
        return modelPoints;
    }

    std::vector<cv::Point3d> modelPointsToWorldPoints(const WorldToModelCoordinates& worldToModel,
                                                      const std::vector<cv::Point2d>& modelPoints,
                                                      double z) {

        const cv::Point3d& invTranslation = - cv::Point3d(worldToModel.translation);
        std::vector<cv::Point3d> worldPoints;
        for (const cv::Point2d& modelPoint : modelPoints) {
            cv::Point3d worldPoint = cv::Point3d(modelPoint.x, modelPoint.y, z) + invTranslation;
            worldPoints.push_back(worldPoint);
        }
        return worldPoints;
    }

    CameraIntrinsics::CameraIntrinsics(const cv::Point2d& focalLength,
                                       const cv::Point2d& principalPoint,
                                       double skew,
                                       const cv::Point3d& radialDistortionCoefficients,
                                       const cv::Point2d& tangentialDistortionCoefficients,
                                       const cv::Point2d& sensorPixelSize) {

        double fx = focalLength.x;
        double fy = focalLength.y;

        double cx = principalPoint.x;
        double cy = principalPoint.y;

        // Format, see here: https://docs.opencv.org/master/d9/d0c/group__calib3d.html#ga3207604e4b1a1758aa66acb6ed5aa65d
        // [ fx skew cx
        //   0  fy   cy
        //   0  0    1  ]
        this->cameraMatrix = (cv::Mat1d(3, 3) << fx, skew, cx, 0, fy, cy, 0, 0, 1);

        double k1 = radialDistortionCoefficients.x;
        double k2 = radialDistortionCoefficients.y;
        double k3 = radialDistortionCoefficients.z;
        double p1 = tangentialDistortionCoefficients.x;
        double p2 = tangentialDistortionCoefficients.y;

        // Format: [k1, k2, p1, p2, k3], see here: https://docs.opencv.org/master/d9/d0c/group__calib3d.html#ga3207604e4b1a1758aa66acb6ed5aa65d
        // Yes, k3 is really supposed to be after p2
        this->distCoeffs = (cv::Mat1d(1, 5) << k1, k2, p1, p2, k3);

        this->sensorSize = sensorPixelSize;
    }

    /**
     * Get rail rectangle in model coordinates
     */
    std::vector<cv::Point2d> getRailRect(double innerTableLength, double innerTableWidth) {
        const cv::Point2d& innerTableCenter { innerTableLength/2, innerTableWidth/2 };
        double railTop = innerTableWidth;
        double railBottom = 0.0;
        double railLeft = 0.0;
        double railRight = innerTableLength;
        return { // In model coordinates
                // Bottom-left
                cv::Point2d { railLeft, railBottom } - innerTableCenter,
                // Top-left
                cv::Point2d { railLeft, railTop } - innerTableCenter,
                // Top-right
                cv::Point2d { railRight, railTop } - innerTableCenter,
                // Bottom-right
                cv::Point2d { railRight, railBottom } - innerTableCenter,
        };
    }

   /**
    * Get table pockets in model coordinates
    */
    std::vector<Pocket> getPockets(double innerTableLength,
                                   double innerTableWidth) {

       // Translation from rail-coordinate system to model-coordinate system
        double railToModelX = innerTableLength/2;
        double railToModelY = innerTableWidth/2;

        double middlePocketRadius = 50;
        double cornerPocketRadius = 50;

        double pocketTop = innerTableWidth - railToModelY;
        double pocketBottom = 0.0 - railToModelY;
        double pocketLeft = 0.0 - railToModelX;
        double pocketRight = innerTableLength - railToModelX;
        double pocketMiddle = innerTableLength/2 - railToModelX;

        return { // In model coordinates
                Pocket { pocketLeft  + 25, pocketBottom + 15, cornerPocketRadius }, // Bottom-left
                Pocket { pocketLeft  + 25, pocketTop    - 15, cornerPocketRadius }, // Top-left
                Pocket { pocketMiddle,        pocketTop    + 15, middlePocketRadius }, // Top-middle
                Pocket { pocketRight - 25, pocketTop    - 15, cornerPocketRadius }, // Top-right
                Pocket { pocketRight - 25, pocketBottom + 15, cornerPocketRadius }, // Bottom-right
                Pocket { pocketMiddle,        pocketBottom - 15, middlePocketRadius }, // Bottom-middle
        };
    }

    inline std::vector<cv::Point> toIntPoints(const std::vector<cv::Point2d>& points) {
        std::vector<cv::Point> intPoints;
        for (auto& imagePoint : points) {
            intPoints.emplace_back((int) imagePoint.x,  (int) imagePoint.y);
        }
        return intPoints;
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
                cornerPositions(markers.bottomLeft - centerOffset), // Marker 0
                cornerPositions(markers.bottomRight - centerOffset), // Marker 1
                cornerPositions(markers.topRight - centerOffset), // Marker 2
                cornerPositions(markers.topLeft - centerOffset), // Marker 3
        };

        cv::Ptr<cv::aruco::Board> board = cv::aruco::Board::create(objPoints, dictionary, ids);
        return board;
    }

    void printCoordinates(const std::string& context,
                          const std::vector<cv::Point2d>& imagePoints,
                          const std::vector<cv::Point3d>& worldPoints,
                          const std::vector<cv::Point2d>& modelPoints) {

#ifdef BILLIARD_DETECTION_DEBUG_PRINT
        for(int i = 0; i < imagePoints.size(); i++) {
            auto& imagePoint = imagePoints[i];
            auto& worldPoint = worldPoints[i];
            auto& modelPoint = modelPoints[i];
            DETECTION_DEBUG("[" << context << "] "
                  << " image point: " << imagePoint
                  << " world point: " << worldPoint
                  << " model point: " << modelPoint
                  << std::endl);
        }
#endif
    }

    inline void drawRailRectLines(cv::Mat& output, const std::vector<cv::Point2d>& points) {

        cv::Scalar color(0, 0, 255);
        int thickness = 1;
        cv::line(output, points[0], points[1], color, thickness);
        cv::line(output, points[1], points[2], color, thickness);
        cv::line(output, points[2], points[3], color, thickness);
        cv::line(output, points[3], points[0], color, thickness);
    }

    inline void drawLines(cv::Mat& output, const std::vector<cv::Point2d>& lines) {

        cv::Scalar color(0, 0, 255);
        int thickness = 1;
        for (int i = 1; i < lines.size(); i+= 2) {
            cv::line(output, lines[i-1], lines[i], color, thickness);
        }
    }

    DetectionConfig configure(const cv::Mat& original,
                              const Table& table,
                              const ArucoMarkers& markers,
                              const CameraIntrinsics& intrinsics) {

        DetectionConfig config;

        cv::Ptr<cv::aruco::Board> board = createArucoBoard(markers);
        CameraToWorldCoordinateSystemConfig cameraToWorld = configure(original, board, intrinsics);

        if (!cameraToWorld.valid) {
            config.valid = false;
            return config;
        }
        config.cameraToWorld = cameraToWorld;

        config.ballPlane = {{0, 0, table.ballDiameter/2 - table.arucoHeightAboveInnerTable}, {0, 0, 1}};

        cv::Vec3d railToModel { table.innerTableLength/2, table.innerTableWidth/2, 0.0 };
        config.worldToModel.translation = table.worldToRail - railToModel;

        std::vector<cv::Point2d> railRect = getRailRect(table.innerTableLength, table.innerTableWidth);
        std::vector<Pocket> pockets = getPockets(table.innerTableLength, table.innerTableWidth);
//        std::vector<Pocket> pockets = table.pockets; // TODO: remove this or line above

        // Build inner table mask
        cv::Mat railMask(cv::Size(original.cols, original.rows), CV_8UC1, cv::Scalar(0));
        cv::Mat pocketMask(cv::Size(original.cols, original.rows), CV_8UC1, cv::Scalar(0));
        cv::Mat innerTableMask(cv::Size(original.cols, original.rows), CV_8UC1, cv::Scalar(0));

        double pixelsPerMillimeter;

        // Rails
        {
            std::vector<cv::Point2d> modelPoints = railRect;
            std::vector<cv::Point3d> worldPoints = modelPointsToWorldPoints(config.worldToModel, modelPoints,
                                                                            table.railWorldPointZComponent);
            std::vector<cv::Point2d> imagePoints = worldPointsToImagePoints(config.cameraToWorld, worldPoints);
            cv::fillConvexPoly(railMask, toIntPoints(imagePoints), cv::Scalar(255));
            railMask.copyTo(innerTableMask);

            printCoordinates("Rail-Rect", imagePoints, worldPoints, modelPoints);
#ifdef BILLIARD_DETECTION_DEBUG_VISUAL
            cv::Mat railOutput = original.clone();
            drawRailRectLines(railOutput, imagePoints);
            cv::resize(railOutput, railOutput, cv::Size(), config.scale, config.scale);
            cv::imshow("rail rectangle", railOutput);
#endif

            // resolutionX = bottom-right - bottom-left
            double resolutionX = (imagePoints[3].x - imagePoints[0].x) * config.scale;
            double tableLength = table.innerTableLength;
            double pixelsPerMillimeterX = resolutionX / tableLength;

            // resolutionY = bottom-left - top-left
            double resolutionY = (imagePoints[0].y - imagePoints[1].y) * config.scale;
            double tableWidth = table.innerTableWidth;
            double pixelsPerMillimeterY = resolutionY / tableWidth;

            DETECTION_DEBUG("[detection:configure] "
                  << " resolutionX: " << resolutionX
                  << " tableLength: " << tableLength
                  << " resolutionY: " << resolutionY
                  << " tableWidth: " << tableWidth
                  << " pixels per millimeter in X/Y: " << pixelsPerMillimeterX << "/" << pixelsPerMillimeterY
                  << std::endl);

            pixelsPerMillimeter = pixelsPerMillimeterX; // Take X because that's the longer axis
            config.ballRadiusInPixel = ceil((table.ballDiameter / 2.0) * pixelsPerMillimeter);
        }

#ifdef BILLIARD_DETECTION_DEBUG_VISUAL
        // Rail segments
        {
            std::vector<cv::Point2d> modelPoints {};
            for (auto& rail : table.railSegments) {
                modelPoints.emplace_back(rail.start.x, rail.start.y);
                modelPoints.emplace_back(rail.end.x, rail.end.y);
            }
            std::vector<cv::Point3d> worldPoints = modelPointsToWorldPoints(config.worldToModel, modelPoints,
                                                                            table.railWorldPointZComponent);
            std::vector<cv::Point2d> imagePoints = worldPointsToImagePoints(config.cameraToWorld, worldPoints);

            cv::Mat railSegmentOutput = original.clone();
            drawLines(railSegmentOutput, imagePoints);
            cv::resize(railSegmentOutput, railSegmentOutput, cv::Size(), config.scale, config.scale);
            cv::imshow("rail segments", railSegmentOutput);
        }
#endif

        // Pockets
        {
            std::vector<cv::Point2d> centers;
            centers.reserve(pockets.size());
            for (auto& pocket : pockets) centers.emplace_back(pocket.x, pocket.y);

            std::vector<cv::Point2d> modelPoints = centers;
            std::vector<cv::Point3d> worldPoints = modelPointsToWorldPoints(config.worldToModel, modelPoints,
                                                                            table.railWorldPointZComponent);
            std::vector<cv::Point2d> imagePoints = worldPointsToImagePoints(config.cameraToWorld, worldPoints);

            for (int i = 0; i < pockets.size(); i++) {
                auto& pocket = pockets[i];
                int pocketPixelRadius = pocket.radius * (pixelsPerMillimeter + 0.1); // TODO: 0.1 because detection
                cv::circle(pocketMask, imagePoints[i], pocketPixelRadius, cv::Scalar{255},
                           cv::LineTypes::FILLED);
                cv::circle(innerTableMask, imagePoints[i], pocketPixelRadius, cv::Scalar{0},
                           cv::LineTypes::FILLED);
            }

#ifdef BILLIARD_DETECTION_DEBUG_VISUAL
            cv::Mat pocketsOutput = original.clone();

            for (int i = 0; i < pockets.size(); i++) {
                auto& pocket = pockets[i];
                int pocketPixelRadius = pocket.radius * pixelsPerMillimeter;
                cv::circle(pocketsOutput, imagePoints[i], pocketPixelRadius, cv::Scalar{0, 0, 255}, 1);
            }

            cv::resize(pocketsOutput, pocketsOutput, cv::Size(), config.scale, config.scale);
            cv::imshow("pockets", pocketsOutput);
#endif
        }

        cv::resize(innerTableMask, innerTableMask, cv::Size(), config.scale, config.scale);
        config.innerTableMask = innerTableMask;
        cv::resize(railMask, railMask, cv::Size(), config.scale, config.scale);
        config.railMask = railMask;

#ifdef BILLIARD_DETECTION_DEBUG_VISUAL
        cv::imshow("Rail mask", railMask);
        cv::imshow("Pocket mask", pocketMask);
        cv::imshow("Inner table mask", innerTableMask);

        cv::Mat inputMaskedByInnerTableMask;
        cv::bitwise_and(original, original, inputMaskedByInnerTableMask, innerTableMask);
        cv::imshow("Input masked by inner table mask", inputMaskedByInnerTableMask);

        cv::Mat invInnerTableMask;
        cv::bitwise_not(innerTableMask, invInnerTableMask);
        cv::Mat innerMaskedDelta;
        cv::bitwise_and(original, original, innerMaskedDelta, invInnerTableMask);
        cv::imshow("Input delta masked by inverse inner table mask", innerMaskedDelta);
#endif
        config.valid = true;
        return config;
    }

    State pixelToModelCoordinates(const DetectionConfig& config, const State& input) {

        std::vector<cv::Point2d> imagePoints;
        for(const auto & ball : input._balls) {
            imagePoints.emplace_back(ball._position.x, ball._position.y);
        }

        std::vector<cv::Point3d> worldPoints = billiard::detection::imagePointsToWorldPoints(config.cameraToWorld, config.ballPlane, imagePoints);
        std::vector<cv::Point2d> modelPoints = billiard::detection::worldPointsToModelPoints(config.worldToModel, worldPoints);

        printCoordinates("Circles", imagePoints, worldPoints, modelPoints);

        State state;
        state._balls.reserve(modelPoints.size());

        for (int i = 0; i < modelPoints.size(); i++) {
            auto& inputBall = input._balls[i];
            auto& point = modelPoints[i];
            Ball ball;
            ball._id = inputBall._id;
            ball._type = inputBall._type;
            ball._position = glm::vec2 {point.x, point.y};
            ball._trackingCount = inputBall._trackingCount;
            state._balls.push_back(ball);
        }
        return state;
    }

}