using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using TMPro;
using UnityEngine.SceneManagement;

public enum StabilizationStatus {
    UNKNOWN,
    STABLE,
    UNSTABLE,
    CUE_BALL_LOST
}

public class StabilizationChange {
    public StabilizationStatus previous;
    public StabilizationStatus current;
}

public class StabilizationController : MonoBehaviour {

	public event Action<StabilizationChange> OnStateStabilizationChange;

    public bool track = false;

    private double maximumCueBallTrackingSquaredDistance = 10*10; // in millimeters^2
	private double maximumStableAverageSquaredDistance   = 10*10; // in millimeters^2
	private int maximumStableUnstableBalls = 0;
	private int maximumStableLostBallCount = 0;

	private CueBallTracking cueBallTracking = new CueBallTracking(1.0f, 2.0f);
	private BallTracking ballTracking = new BallTracking(10*10, 1.0f, 2.0f, 0.25f, 0.25f);
	private StabilizationStatus currentStatus = StabilizationStatus.UNKNOWN;
	private DateTime? lastStateChange = null;
	private float lastAverageBallCount = -1;

    public void Start() {
        AnimationService.OnStateReceived += stateChanged;
    }

    void OnDestroy() {
        AnimationService.OnStateReceived -= stateChanged;
    }

    public void stateChanged(RootState state) {

        if (!track) {
            lastStateChange = null;
            currentStatus = StabilizationStatus.UNKNOWN;
            cueBallTracking.clear();
            ballTracking.clear();
            return;
        }

        DateTime now = DateTime.Now;
        float deltaTime = 0.0f;
        if (lastStateChange != null) {
            double milliseconds = (now - lastStateChange.Value).TotalMilliseconds;
            deltaTime = (float) (milliseconds / 1000.0);
        }

        Debug.Log("State change: deltaTime=" + deltaTime);

        List<BallState> balls = new List<BallState>();
        foreach (BallState ball in state.balls) {
            balls.Add(new BallState {
                id = ball.id,
                type = ball.type,
                position = new Vec2 { x = ball.position.x * 1000.0, y = ball.position.y * 1000.0 }
            });
        }

        lastStateChange = now;
        StabilizationStatus previousStatus = currentStatus;
        StabilizationStatus status = stateChanged(balls, deltaTime);
        if (previousStatus != status) {
            Debug.Log("Stabilization changed from " + previousStatus + " to " + status + ", deltaTime=" + deltaTime);
            OnStateStabilizationChange?.Invoke(new StabilizationChange { previous = previousStatus, current = status });
        }

    }

    public StabilizationStatus stateChanged(List<BallState> balls, float deltaTime) {

        string agent = "[stateChanged] ";

        List<BallState> cueBalls = findBalls(balls, "WHITE");
        BallState cueBall = null;

        if (cueBalls.Count == 1) {

            // One cue ball found
            cueBall = cueBalls[0];

        } else if (cueBalls.Count > 1) {

            // Too many cue balls found, try finding it around the previous position
            cueBall = tryFindingCueBall(cueBalls);
            if (cueBall == null) {
                // Cue ball could not be found around previous position
                currentStatus = StabilizationStatus.CUE_BALL_LOST;
                return currentStatus;
            }
        } else {
            // No cue ball found
            Debug.Log(agent + "No cue ball found");
            // Do not change status:
            // State probably is UNSTABLE because in order for the cueball to go missing, there had to be movement of any kind.
            // Once the cue ball is put back or is detected again, the state will begin to stabilize.
            return currentStatus;
        }

        cueBallTracking.add(cueBall.position, deltaTime);
        ballTracking.add(balls, deltaTime);

        string cueBallDebugOutput = "";

        if (cueBallTracking.warmedUp() && ballTracking.warmedUp()) {

            Vec2 averageCueBallMovement = cueBallTracking.averageMovement();
            double avgCueBallSquaredDistance = Vec2.dot(averageCueBallMovement, averageCueBallMovement);

            int totalLostBallsCount = ballTracking.totalLostCount;
            List<string> unstableBallIds = new List<string>();
            TimedState<Balls> currentTracking = ballTracking.history.First();
            foreach (TrackedBall trackedBall in currentTracking.state.balls) {

                double averageBallSquaredDistance = Vec2.dot(trackedBall.averageMovement, trackedBall.averageMovement);
                if (averageBallSquaredDistance > maximumStableAverageSquaredDistance) {
                    Debug.Log("ballid = " + trackedBall.id + " averageBallSquaredDistance=" + averageBallSquaredDistance + " maximumStableAverageSquaredDistance=" + maximumStableAverageSquaredDistance);
                    unstableBallIds.Add(trackedBall.id);
                }
            }
            int stableBallCountChange = ballTracking.getStableBallCountChange();
            int stableBallCount = ballTracking.getStableBallCount();
            int currentBallCount = ballTracking.getCurrentBallCount();
            float currentAverageBallCount = ballTracking.averageBallCount();
//                 float lastAverageBallCount = ballTracking.oldestAverageBallCount(); // TODO: remove?
            float lastAverageBallCount = ballTracking.previousAverageBallCount(); // TODO: remove?
            float totalAverageBallCountChange = ballTracking.totalAverageBallCountChange;

            bool cueBallStable = avgCueBallSquaredDistance < maximumStableAverageSquaredDistance;
            bool tooManyUnstableBalls = unstableBallIds.Count > maximumStableUnstableBalls;
            bool tooManyLostBalls = totalLostBallsCount > maximumStableLostBallCount;
//                 bool averageBallCountChangedTooMuch = lastAverageBallCount > 0 && Math.Abs(lastAverageBallCount - currentAverageBallCount) > 0.5;
//                 bool averageBallCountChangedTooMuch = Math.Abs(lastAverageBallCount - currentAverageBallCount) > 0.5;
            bool averageBallCountChangedTooMuch = Math.Abs(totalAverageBallCountChange) > 0.5;
            // Consider positive values only, because lost balls are already handled with a different metric
            bool newBallsWereAdded = stableBallCountChange >= 1;
            bool countEqualsStableCount = currentBallCount == stableBallCount;


//                 if (cueBallStable && !tooManyUnstableBalls && !tooManyLostBalls) {
//                 if (cueBallStable && !tooManyUnstableBalls && !tooManyLostBalls && !averageBallCountChangedTooMuch) {
            if (cueBallStable && !tooManyUnstableBalls && !tooManyLostBalls && !newBallsWereAdded) { // TODO: try out newBallsWereAdded
//                    cueBallDebugOutput += "Cue ball stable ("
//                        + String.Format("{0:0.000}", Math.Sqrt(avgCueBallSquaredDistance)) + "mm, "
//                        + "unstable=" + unstableBallIds.Count + ", "
//                        + "lost=" + totalLostBallsCount + ", "
//                        + currentAverageBallCount
// //                        + "/"
// //                        + lastAverageBallCount
//                        + "/"
//                        + totalAverageBallCountChange
//                        + ", "
//                        + "added=" + stableBallCountChange
//                        + ", "
//                        + "stable count=" + stableBallCount
//                        + "/" + currentBallCount
//                        + ") ";
               if (countEqualsStableCount) {
                   cueBallDebugOutput += "Cue ball stable ("
                                      + String.Format("{0:0.000}", Math.Sqrt(avgCueBallSquaredDistance)) + "mm, "
                                      + "unstable=" + unstableBallIds.Count + ", "
                                      + "lost=" + totalLostBallsCount + ", "
                                      + currentAverageBallCount
               //                        + "/"
               //                        + lastAverageBallCount
                                      + "/"
                                      + totalAverageBallCountChange
                                      + ", "
                                      + "added=" + stableBallCountChange
                                      + ", "
                                      + "stable count=" + stableBallCount
                                      + "/" + currentBallCount
                                      + ") ";
                   currentStatus = StabilizationStatus.STABLE;
               } else {
                   // Do not change status because not all balls have stabilized,
                   // that may be because of ghost balls appearing in a stable state, in which case we do not want to change to UNSTABLE,
                   // or it may be because some ball is moving and could not be tracked and is therefore classified as a ghost in an unstable state, in which case we do not want to change to STABLE.
                   cueBallDebugOutput += "Cue ball indecisive ("
                                      + String.Format("{0:0.000}", Math.Sqrt(avgCueBallSquaredDistance)) + "mm, "
                                      + "unstable=" + unstableBallIds.Count + ", "
                                      + "lost=" + totalLostBallsCount + ", "
                                      + currentAverageBallCount
                 //                      + "/"
                 //                      + lastAverageBallCount
                                      + "/"
                                      + totalAverageBallCountChange
                                      + ", "
                                      + "added=" + stableBallCountChange
                                      + ", "
                                      + "stable count=" + stableBallCount
                                      + "/" + currentBallCount
                                      + ") ";
               }
            } else {
               cueBallDebugOutput += "Cue ball unstable ("
                   + String.Format("{0:0.000}", Math.Sqrt(avgCueBallSquaredDistance)) + "mm, "
                   + "unstable=[ "
                   + string.Join(", ", unstableBallIds)
                   + "], "
                   + "lost=" + totalLostBallsCount + ", "
                   + currentAverageBallCount
//                        + "/"
//                        + lastAverageBallCount
                   + "/"
                   + totalAverageBallCountChange
                   + ", "
                   + "added=" + stableBallCountChange
                   + ", "
                   + "stable count=" + stableBallCount
                   + "/" + currentBallCount
                   + ") ";
               currentStatus = StabilizationStatus.UNSTABLE;
            }

//                 if (lastAverageBallCount < 0 || averageBallCountChangedTooMuch) {
//                     lastAverageBallCount = currentAverageBallCount;
//                 }

        } else {
            cueBallDebugOutput += "Cue ball indecisive (not warmed up yet)" + " ";
            // do not change status
        }

        if (cueBallDebugOutput.Length > 0) {
            Debug.Log(agent + cueBallDebugOutput + ballTracking.debugOutput);
        }

        return currentStatus;
    }

    private BallState tryFindingCueBall(List<BallState> cueBalls) {

        string agent = "[stateChanged] ";

        if (cueBallTracking.history.Count > 0) {
            TimedState<Movement> lastCueBall = cueBallTracking.history.First();

            var (nearestBall, squaredDistance) = findNearestBall(cueBalls, lastCueBall.state.position);
            if (nearestBall != null) {
                // Potential match found
                if (squaredDistance < maximumCueBallTrackingSquaredDistance) {
                    Debug.Log(agent + "Too many cue balls found: " + cueBalls.Count + " but found closest: " + nearestBall.position + " (" + Math.Sqrt(squaredDistance) + "mm)");
                    return nearestBall;
                } else {
                    Debug.Log(agent + "Too many cue balls found: " + cueBalls.Count);
                    return null;
                }
            } else {
                return null;
            }
        } else {
            Debug.Log(agent + "Too many cue balls found: " + cueBalls.Count);
            return null;
        }
    }

    private class CueBallTracking {

        // Minimum timespan of recorded history
        private float minTime = 0.0f;   // in seconds
        // Maximum timespan of recorded history
        private float maxTime = 0.0f;   // in seconds

        // Newest entry is first, oldest is last
        public LinkedList<TimedState<Movement>> history = new LinkedList<TimedState<Movement>>();

        // Total timespan of recorded history
        private float totalTime = 0.0f; // in seconds
        // Total movement of recorded history
        public Vec2 totalMovement = new Vec2 {x = 0.0, y = 0.0};

        public CueBallTracking(float minTime, float maxTime) {
            this.minTime = minTime;
            this.maxTime = maxTime;
        }

        public bool warmedUp() {
            return totalTime > minTime;
        }

        public void add(Vec2 position, float deltaTime) {
            Vec2 moved = new Vec2 {x = 0.0, y = 0.0};

            if (history.Count > 0) {
                Vec2 previousPosition = history.First().state.position;
                moved = position - previousPosition;
            }

            history.AddFirst(new TimedState<Movement> {
                state = new Movement { position = position, movement = moved },
                deltaTime = deltaTime
            });
            totalTime += deltaTime;
            totalMovement += moved;

            // Remove old entries
            while (totalTime > maxTime) {
                TimedState<Movement> oldest = history.Last();
                totalTime -= oldest.deltaTime;
                totalMovement -= oldest.state.movement;
                history.RemoveLast();
            }
        }

        public Vec2 averageMovement() {
            return totalMovement / (float) history.Count;
        }

        public void clear() {
            history = new LinkedList<TimedState<Movement>>();
            totalTime = 0.0f;
            totalMovement = new Vec2 {x = 0.0, y = 0.0};
        }

    }

    private class BallTracking {

        // Minimum timespan of recorded history
        private float minTime = 0.0f;   // in seconds
        // Maximum timespan of recorded history
        private float maxTime = 0.0f;   // in seconds
        // Maximum squared distance, that a ball may move, in order to still be tracked.
        private float maxTrackingSquaredDistance = 0.0f; // in millimeters^2
        // Minimal duration that a ball has to be tracked successfully before being considered lost, when tracking fails
        private float minTrackedDurationBeforeLost = 0.0f; // in seconds
        // Minimal duration that a ball has to be tracked successfully before being counted as a ball in the total of balls present on the table
        private float minTrackedDurationBeforeCounted = 0.0f; // in seconds

        // Newest entry is first, oldest is last
        public LinkedList<TimedState<Balls>> history = new LinkedList<TimedState<Balls>>();

        // Total timespan of recorded history
        private float totalTime = 0.0f; // in seconds
        // Total number of lost balls in recorded history
        public int totalLostCount = 0;
        // Total number of balls in recorded history
        public int totalBallCount = 0;

        public float totalAverageBallCountChange = 0.0f;

        public string debugOutput = "";

        public BallTracking(float maxTrackingSquaredDistance,
                            float minTime,
                            float maxTime,
                            float minTrackedDurationBeforeLost,
                            float minTrackedDurationBeforeCounted) {
            this.maxTrackingSquaredDistance = maxTrackingSquaredDistance;
            this.minTime = minTime;
            this.maxTime = maxTime;
            this.minTrackedDurationBeforeLost = minTrackedDurationBeforeLost;
            this.minTrackedDurationBeforeCounted = minTrackedDurationBeforeCounted;
        }

        public bool warmedUp() {
            return totalTime > minTime;
        }

        public float averageBallCount() {
            return (float) totalBallCount / (float) history.Count;
        }

        public int getStableBallCount() {
            return history.First.Value.state.stableBallCount;
        }

        public int getCurrentBallCount() {
            return history.First.Value.state.balls.Count;
        }

        public int getStableBallCountChange() {
            LinkedListNode<TimedState<Balls>> newestNode = history.First;
            LinkedListNode<TimedState<Balls>> secondNewestNode = newestNode.Next;
            return newestNode.Value.state.stableBallCount - secondNewestNode.Value.state.stableBallCount;
        }

        public float oldestAverageBallCount() {
            // Last history entries average ball count is the average ball count from N seconds before that last entry
            return history.Last().state.averageBallCount;
        }

        public float previousAverageBallCount() {
            return history.ElementAt(history.Count / 2).state.averageBallCount;
        }

        public void add(List<BallState> balls, float deltaTime) {

            debugOutput = "";

            if (history.Count > 0) {
                TimedState<Balls> previousState = history.First();

                List<TrackedBall> result = new List<TrackedBall>();
                int lostCount = 0;
                int stableBallCount = 0;

                foreach (BallState ball in balls) {

                    if (ball.type == "WHITE") {
                        continue; // Don't track cue ball here
                    }

                    TrackedBall previousBall = null;
                    Vec2 movement = new Vec2 { x = 0.0, y = 0.0 };
                    Vec2 averageMovement = new Vec2 { x = 0.0, y = 0.0 };
                    float trackedDuration = 0.0f;

                    var (nearestBall, squaredDistance) = findNearestBall(previousState.state.balls, ball.position);
                    if (nearestBall != null) {
                        // Potential match found
                        if (squaredDistance < maxTrackingSquaredDistance) {
                            // Seems good
                            previousBall = nearestBall;
                            movement = ball.position - nearestBall.position;
                            trackedDuration = previousBall.trackedDuration + deltaTime;

                            debugOutput += ball.id + " tracked ("
                                + previousBall.id + ", "
                                + String.Format("{0:0.000}", Math.Sqrt(Vec2.dot(movement, movement))) + "mm, "
                                + String.Format("{0:0.00}", trackedDuration) + "s";
                            averageMovement = calculateAverageMovement(ball.position, previousBall);
                        } else {
                            // Too far away, ball is either new or could not be tracked
                            previousBall = null;
                            movement = new Vec2 { x = 0.0, y = 0.0 };
                            averageMovement = new Vec2 { x = 0.0, y = 0.0 };
                            trackedDuration = 0.0f;

                            debugOutput += ball.id + " not tracked ("
                                + String.Format("{0:0.000}", Math.Sqrt(squaredDistance)) + "mm" + " > "
                                + String.Format("{0:0.000}", Math.Sqrt(maxTrackingSquaredDistance)) + ", "
                                + "moved ("
                                + (ball.position.x - nearestBall.position.x) + ","
                                + (ball.position.y - nearestBall.position.y) + ")"
                                + ")" + " ";
                        }
                    } else {
                        // No ball found, happens only when there were no balls on the table
                        previousBall = null;
                        movement = new Vec2 { x = 0.0, y = 0.0 };
                        averageMovement = new Vec2 { x = 0.0, y = 0.0 };
                        trackedDuration = 0.0f;

                        debugOutput += ball.id + " not tracked (no balls)" + " ";
                    }

                    TrackedBall trackedBall = new TrackedBall {
                       id = ball.id,
                       previous = previousBall,
                       next = null,
                       trackedDuration = trackedDuration,
                       type = ball.type,
                       position = ball.position,
                       movement = movement,
                       averageMovement = averageMovement
                    };
                    if (trackedBall.previous != null) {
                        // Update previous balls link to current ball
                        trackedBall.previous.next = trackedBall;
                    }

                    if (trackedBall.trackedDuration > minTrackedDurationBeforeCounted) {
                        // Count ball in total number of balls present on the table only if it has been successfully tracked for some time
                        stableBallCount++;
                    }

                    result.Add(trackedBall);
                }

                // Go through balls from previous state
                foreach (TrackedBall ball in previousState.state.balls) {
                    if (ball.next == null) {
                        // Ball could not be found in current frame, maybe it was lost?
                        if (ball.trackedDuration > minTrackedDurationBeforeLost) {
                            // Ball was tracked long enough to be considered a lost ball
                            // (Balls not tracked long enough may just be detection errors)
                            lostCount++;

                            debugOutput += ball.id + " lost ("
                                        + String.Format("{0:0.00}", ball.trackedDuration) + "s" + ") ";
                        } else {
                            // Ball may have been a ghost
                            // (a ball that is detected in one frame and immediately disappears in the next frame)
                            debugOutput += ball.id + " ghost? ("
                                        + String.Format("{0:0.00}", ball.trackedDuration) + "s" + ") ";
                        }
                    }
                }

                float currentAverageBallCount = averageBallCount();
                float averageBallCountChange = previousState.state.averageBallCount - currentAverageBallCount;

                history.AddFirst(new TimedState<Balls> {
                    state = new Balls {
                        balls = result,
                        lostCount = lostCount,
                        stableBallCount = stableBallCount,
                        averageBallCount = currentAverageBallCount,
                        averageBallCountChange = averageBallCountChange
                    },
                    deltaTime = deltaTime
                });

                totalTime += deltaTime;
                totalLostCount += lostCount;
                totalBallCount += result.Count;
                totalAverageBallCountChange += averageBallCountChange;

            } else {
                // First history entry
                addFirstEntry(balls, deltaTime);
            }

            cleanup();
        }

        private void addFirstEntry(List<BallState> balls, float deltaTime) {
            debugOutput += "First entry";

            List<TrackedBall> result = new List<TrackedBall>();
            foreach (BallState ball in balls) {

                if (ball.type == "WHITE") {
                    continue;
                }

                result.Add(new TrackedBall {
                    id = ball.id,
                    previous = null,
                    next = null,
                    trackedDuration = 0.0f,
                    type = ball.type,
                    position = ball.position,
                    movement = new Vec2 { x = 0.0, y = 0.0 },
                    averageMovement = new Vec2 { x = 0.0, y = 0.0 }
                });
            }

            history.AddFirst(new TimedState<Balls> {
                state = new Balls {
                    balls = result,
                    lostCount = 0,
                    averageBallCount = result.Count,
                    averageBallCountChange = 0
                },
                deltaTime = deltaTime
            });

            totalTime += deltaTime;
            totalBallCount += result.Count;
            // totalAverageBallCountChange does not change
        }

        private void cleanup() {
            // Remove old entries
            while (totalTime > maxTime) {
                LinkedListNode<TimedState<Balls>> last = history.Last;
                TimedState<Balls> oldest = last.Value;
                totalTime -= oldest.deltaTime;
                totalLostCount -= oldest.state.lostCount;
                totalBallCount -= oldest.state.balls.Count;
                totalAverageBallCountChange -= oldest.state.averageBallCountChange;

                // Unlink removed entry
                if (last.Previous != null) {
                    TimedState<Balls> secondOldest = last.Previous.Value;
                    foreach(TrackedBall ball in secondOldest.state.balls) {
                        ball.previous = null;
                    }
                }
                foreach(TrackedBall ball in oldest.state.balls) {
                    ball.next = null;
                }

                history.RemoveLast();
            }
        }

        private Vec2 calculateAverageMovement(Vec2 currentPos, TrackedBall previousBall) {
            Vec2 currentPosition = currentPos;
            Vec2 totalMovement = new Vec2 { x = 0.0, y = 0.0 };
            float movements = 0.0f;
            TrackedBall node = previousBall;
            while (node != null) {

                Vec2 previousPosition = node.position;
                Vec2 moved = currentPosition - previousPosition;
                totalMovement += moved;
                movements++;

                currentPosition = node.position;
                node = node.previous;
            }
            Vec2 averageMovement = totalMovement / movements;

            debugOutput += ", " + String.Format("{0:0.000}", Math.Sqrt(Vec2.dot(averageMovement, averageMovement))) + "mm" + ", " + ((int) movements) + ")" + " ";

            return averageMovement;
        }

        private (TrackedBall, double) findNearestBall(List<TrackedBall> balls, Vec2 position) {
            TrackedBall nearest = null;
            double minSquaredDistance = 1000000.0;
            foreach (TrackedBall ball in balls) {
                Vec2 delta = position - ball.position;
                double squaredDistance = Vec2.dot(delta, delta);

                if (nearest == null || squaredDistance < minSquaredDistance) {
                    nearest = ball;
                    minSquaredDistance = squaredDistance;
                }
            }
            return (nearest, minSquaredDistance);
        }

        public void clear() {
            history = new LinkedList<TimedState<Balls>>();
            totalTime = 0.0f;
            totalLostCount = 0;
            totalBallCount = 0;
        }
    }

    private class TimedState<T> {
    	public T state;
    	// Time period between this entry and previous entry
    	public float deltaTime;
    }

    private class Movement {
        // Current position
        public Vec2 position;
        // Movement since previous entry
        public Vec2 movement;
    }

    public class Balls {
        public List<TrackedBall> balls;
        public int lostCount = 0;
        public int stableBallCount = 0;
        // Average ball count over several successful trackings, once written, never updated
        public float averageBallCount;

        public float averageBallCountChange;
    }

    public class TrackedBall {
        public string id;
        public string type;
        public Vec2 position;
        // Previous ball if tracking succeeded, null if tracking failed
        public TrackedBall previous;
        // Next ball if tracking succeeded (in the future), null if tracking failed
        public TrackedBall next;
        // Amount of time the ball was successfully tracked
        public float trackedDuration;
        // Movement since previous entry or zero-vector if tracking failed
        public Vec2 movement;
        // Average movement over several successful trackings, once written, never updated
        public Vec2 averageMovement;
    }

    public List<BallState> findBalls(List<BallState> balls, string type) {
        List<BallState> result = new List<BallState>();
        foreach (BallState ball in balls) {
            if (ball.type == type) {
                result.Add(ball);
            }
        }
        return result;
    }

    public (BallState, double) findNearestBall(List<BallState> balls, Vec2 position) {
        BallState nearest = null;
        double minSquaredDistance = 1000000.0;
        foreach (BallState ball in balls) {
            double sqDistance = squaredDistance(position, ball.position);

            if (nearest == null || sqDistance < minSquaredDistance) {
                nearest = ball;
                minSquaredDistance = sqDistance;
            }
        }
        return (nearest, minSquaredDistance);
    }

    private double squaredDistance(Vec2 p1, Vec2 p2) {
        Vec2 delta = p1 - p2;
        return Vec2.dot(delta, delta);
    }
}