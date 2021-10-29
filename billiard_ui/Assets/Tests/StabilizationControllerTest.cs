using System;
using System.Collections;
using System.Collections.Generic;
using NUnit.Framework;
using UnityEngine;
using UnityEngine.TestTools;

namespace Tests
{
    public class StabilizationControllerTest
    {

        [Test]
        public void stateChanged_should_stay_stable_if_positions_change_slightly_randomly_over_time() {

            StabilizationController controller = new StabilizationController();
            controller.track = true;

            BallState white = new BallState { type = "WHITE", id = "WHITE1", position = new Vec2 { x = 0, y = 0} };
            BallState red = new BallState { type = "RED", id = "RED1", position = new Vec2 { x = 400, y = 300} };

            List<BallState> balls = new List<BallState> { red, white };
            float maxPerturbation = 3; // in millimeters (in both directions)
            int iterations = 1000;
            float timeStep = 0.1f;

            // Warmup
            Debug.Log("Warmup");
            controller.stateChanged(balls, 0.0f);
            StabilizationStatus lastStatus = StabilizationStatus.UNKNOWN;
            int warmupCounter = 0;
            while ((lastStatus != StabilizationStatus.STABLE || warmupCounter < 30) && warmupCounter < 10000) {
                lastStatus = controller.stateChanged(perturb(balls, maxPerturbation), timeStep);
                warmupCounter++;
            }
            Assert.AreEqual(StabilizationStatus.STABLE, lastStatus);

            Debug.Log("Warmup finished, start test");

            // Now perturb and check
            controller.stateChanged(balls, timeStep);
            for (int i = 0; i < iterations; i++) {
                StabilizationStatus status = controller.stateChanged(perturb(balls, maxPerturbation), timeStep);
                Assert.AreEqual(StabilizationStatus.STABLE, status);
            }
        }

        [Test]
        public void stateChanged_should_stay_stable_if_ghost_balls_appear() {

            StabilizationController controller = new StabilizationController();
            controller.track = true;

            BallState white = new BallState { type = "WHITE", id = "WHITE1", position = new Vec2 { x = 0, y = 0} };
            BallState red = new BallState { type = "RED", id = "RED1", position = new Vec2 { x = 400, y = 300} };
            BallState ghost1 = new BallState { type = "GREEN", id = "GHOST1", position = new Vec2 { x = -200, y = 150} };
            BallState ghost2 = new BallState { type = "GREEN", id = "GHOST2", position = new Vec2 { x = -300, y = -400} };

            List<BallState> balls = new List<BallState> { red, white };
            float maxPerturbation = 3; // in millimeters (in both directions)
            int iterations = 10;
            float timeStep = 0.1f;

            // Warmup
            Debug.Log("Warmup ------------------------------------------------------------");
            controller.stateChanged(balls, 0.0f);
            StabilizationStatus lastStatus = StabilizationStatus.UNKNOWN;
            int warmupCounter = 0;
            while ((lastStatus != StabilizationStatus.STABLE || warmupCounter < 30) && warmupCounter < 10000) {
                lastStatus = controller.stateChanged(perturb(balls, maxPerturbation), timeStep);
                warmupCounter++;
            }
            Assert.AreEqual(StabilizationStatus.STABLE, lastStatus);

            Debug.Log("Warmup finished, start test ------------------------------------------------------------");

            controller.stateChanged(perturb(new List<BallState> { red, white, ghost1 }, maxPerturbation), timeStep);
            for (int i = 0; i < iterations; i++) {
                lastStatus = controller.stateChanged(perturb(balls, maxPerturbation), timeStep);
            }
            Assert.AreEqual(StabilizationStatus.STABLE, lastStatus);

            controller.stateChanged(perturb(new List<BallState> { red, ghost2, white }, maxPerturbation), timeStep);
            for (int i = 0; i < iterations; i++) {
                lastStatus = controller.stateChanged(perturb(balls, maxPerturbation), timeStep);
            }
            Assert.AreEqual(StabilizationStatus.STABLE, lastStatus);
        }

        [Test]
        public void stateChanged_should_handle_slow_moving_balls() {

            StabilizationController controller = new StabilizationController();
            controller.track = true;

            // Note that the red ball should roll further than the white one
            BallState white = new BallState { type = "WHITE", id = "WHITE1", position = new Vec2 { x = 0, y = 0} };
            Vec2 whiteTarget = new Vec2 { x = 150, y = 0};
            Vec2 whiteToTarget = whiteTarget - white.position;
            Vec2 whiteDirection = (whiteTarget - white.position).normalized();
            BallState red = new BallState { type = "RED", id = "RED1", position = new Vec2 { x = 200, y = 0} };
            Vec2 redTarget = new Vec2 { x = 600, y = 0} - red.position;
            Vec2 redToTarget = redTarget - red.position;
            Vec2 redDirection = (redTarget - red.position).normalized();
            BallState blue = new BallState { type = "BLUE", id = "BLUE1", position = new Vec2 { x = -400, y = -250} };
            BallState red2 = new BallState { type = "RED", id = "RED2", position = new Vec2 { x = 650, y = 50} };

            float timeStep = 0.1f;
            double step = 4.0; // In Millimeters
            float maxPerturbation = 3; // in millimeters (in both directions)

            Debug.Log("No ball moving yet ------------------------------------------------------------");
            StabilizationStatus lastStatus = StabilizationStatus.UNKNOWN;
            controller.stateChanged(new List<BallState> { red, white, blue, red2 }, 0.0f);
            for (int i = 0; i < 2.0f / timeStep + 5; i++) {
                List<BallState> balls = new List<BallState> { red, white, blue, red2 };
                StabilizationStatus status = controller.stateChanged(perturb(balls, maxPerturbation), timeStep);
                if (lastStatus == StabilizationStatus.STABLE) {
                    // As soon as status has changed to STABLE, it should not change anymore for stability purposes.
                    Assert.AreEqual(StabilizationStatus.STABLE, status);
                }
                lastStatus = status;
            }
            Assert.AreEqual(StabilizationStatus.STABLE, lastStatus);
            Debug.Log("White starts moving, Red is stationary ------------------------------------------------------------");
            for (double i = 0.0; i < whiteToTarget.length(); i += step) {
                List<BallState> balls = new List<BallState> { red, positionedAt(white, white.position + whiteDirection * i), perturb(blue, maxPerturbation), perturb(red2, maxPerturbation) };
                StabilizationStatus status = controller.stateChanged(balls, timeStep);
                if (lastStatus == StabilizationStatus.UNSTABLE) {
                    // As soon as status has changed to UNSTABLE, it should not change anymore for stability purposes.
                    Assert.AreEqual(StabilizationStatus.UNSTABLE, status);
                }
                lastStatus = status;
            }
            Assert.AreEqual(StabilizationStatus.UNSTABLE, lastStatus);
            Debug.Log("White is stationary, Red starts moving ------------------------------------------------------------");
            for (double i = 0.0; i < redToTarget.length(); i += step) {
                List<BallState> balls = new List<BallState> { positionedAt(red, red.position + redDirection * i), positionedAt(white, whiteTarget), perturb(blue, maxPerturbation), perturb(red2, maxPerturbation) };
                StabilizationStatus status = controller.stateChanged(balls, timeStep);
                Assert.AreEqual(StabilizationStatus.UNSTABLE, status);
                lastStatus = status;
            }
            Assert.AreEqual(StabilizationStatus.UNSTABLE, lastStatus);
            Debug.Log("No ball moving anymore ------------------------------------------------------------");
            for (int i = 0; i < 2.0f / timeStep + 5; i++) {
                List<BallState> balls = new List<BallState> { positionedAt(red, redTarget), positionedAt(white, whiteTarget), blue, red2 };
                StabilizationStatus status = controller.stateChanged(perturb(balls, maxPerturbation), timeStep);
                if (lastStatus == StabilizationStatus.STABLE) {
                    // As soon as status has changed to STABLE, it should not change anymore for stability purposes.
                    Assert.AreEqual(StabilizationStatus.STABLE, status);
                }
                lastStatus = status;
            }
            Assert.AreEqual(StabilizationStatus.STABLE, lastStatus);
        }

        [Test]
        public void stateChanged_should_handle_ball_rolling_into_pocket() {

            StabilizationController controller = new StabilizationController();
            controller.track = true;

            // Note that the red ball should roll further than the white one
            BallState white = new BallState { type = "WHITE", id = "WHITE1", position = new Vec2 { x = 0, y = 0} };
            Vec2 whiteTarget = new Vec2 { x = 150, y = 0};
            Vec2 whiteToTarget = whiteTarget - white.position;
            Vec2 whiteDirection = (whiteTarget - white.position).normalized();
            BallState red = new BallState { type = "RED", id = "RED1", position = new Vec2 { x = 200, y = 0} };
            Vec2 redTarget = new Vec2 { x = 940, y = 440} - red.position;
            Vec2 redToTarget = redTarget - red.position;
            Vec2 redDirection = (redTarget - red.position).normalized();
            BallState blue = new BallState { type = "BLUE", id = "BLUE1", position = new Vec2 { x = -400, y = -250} };
            BallState red2 = new BallState { type = "RED", id = "RED2", position = new Vec2 { x = 880, y = 380} };

            float timeStep = 0.1f;
            double step = 8.0; // In Millimeters
            float maxPerturbation = 3; // in millimeters (in both directions)

            Debug.Log("No ball moving yet ------------------------------------------------------------");
            StabilizationStatus lastStatus = StabilizationStatus.UNKNOWN;
            controller.stateChanged(new List<BallState> { red, white, blue, red2 }, 0.0f);
            for (int i = 0; i < 2.0f / timeStep + 5; i++) {
                List<BallState> balls = new List<BallState> { red, white, blue, red2 };
                StabilizationStatus status = controller.stateChanged(perturb(balls, maxPerturbation), timeStep);
                if (lastStatus == StabilizationStatus.STABLE) {
                    // As soon as status has changed to STABLE, it should not change anymore for stability purposes.
                    Assert.AreEqual(StabilizationStatus.STABLE, status);
                }
                lastStatus = status;
            }
            Assert.AreEqual(StabilizationStatus.STABLE, lastStatus);
            Debug.Log("White starts moving, Red is stationary ------------------------------------------------------------");
            for (double i = 0.0; i < whiteToTarget.length(); i += step) {
                List<BallState> balls = new List<BallState> { red, positionedAt(white, white.position + whiteDirection * i), perturb(blue, maxPerturbation), perturb(red2, maxPerturbation) };
                StabilizationStatus status = controller.stateChanged(balls, timeStep);
                if (lastStatus == StabilizationStatus.UNSTABLE) {
                    // As soon as status has changed to UNSTABLE, it should not change anymore for stability purposes.
                    Assert.AreEqual(StabilizationStatus.UNSTABLE, status);
                }
                lastStatus = status;
            }
            Assert.AreEqual(StabilizationStatus.UNSTABLE, lastStatus);
            Debug.Log("White is stationary, Red starts moving ------------------------------------------------------------");
            for (double i = 0.0; i < redToTarget.length(); i += step) {
                List<BallState> balls = new List<BallState> { positionedAt(red, red.position + redDirection * i), positionedAt(white, whiteTarget), perturb(blue, maxPerturbation), perturb(red2, maxPerturbation) };
                StabilizationStatus status = controller.stateChanged(balls, timeStep);
                Assert.AreEqual(StabilizationStatus.UNSTABLE, status);
                lastStatus = status;
            }
            Assert.AreEqual(StabilizationStatus.UNSTABLE, lastStatus);
            Debug.Log("No ball moving anymore, red is in pocket  ------------------------------------------------------------");
            for (int i = 0; i < 2.0f / timeStep + 5; i++) {
                List<BallState> balls = new List<BallState> { positionedAt(white, whiteTarget), blue, red2 };
                StabilizationStatus status = controller.stateChanged(perturb(balls, maxPerturbation), timeStep);
                if (lastStatus == StabilizationStatus.STABLE) {
                    // As soon as status has changed to STABLE, it should not change anymore for stability purposes.
                    Assert.AreEqual(StabilizationStatus.STABLE, status);
                }
                lastStatus = status;
            }
            Assert.AreEqual(StabilizationStatus.STABLE, lastStatus);
        }

        [Test]
        public void stateChanged_should_handle_fast_moving_balls() {

            StabilizationController controller = new StabilizationController();
            controller.track = true;

            // Note that the red ball should roll further than the white one
            BallState white = new BallState { type = "WHITE", id = "WHITE1", position = new Vec2 { x = -200, y = 0} };
            Vec2 whiteTarget = new Vec2 { x = 100, y = 0};
            Vec2 whiteToTarget = whiteTarget - white.position;
            Vec2 whiteDirection = (whiteTarget - white.position).normalized();
            BallState red = new BallState { type = "RED", id = "RED1", position = new Vec2 { x = -150, y = 0} };
            Vec2 redTarget = new Vec2 { x = 900, y = 0} - red.position;
            Vec2 redToTarget = redTarget - red.position;
            Vec2 redDirection = (redTarget - red.position).normalized();
            BallState blue = new BallState { type = "BLUE", id = "BLUE1", position = new Vec2 { x = -400, y = -250} };

            float timeStep = 0.1f;
            double step = 50.0; // In Millimeters
            float maxPerturbation = 3; // in millimeters (in both directions)

            Debug.Log("No ball moving yet ------------------------------------------------------------");
            StabilizationStatus lastStatus = StabilizationStatus.UNKNOWN;
            controller.stateChanged(new List<BallState> { red, white, blue }, 0.0f);
            for (int i = 0; i < 2.0f / timeStep + 5; i++) {
                List<BallState> balls = new List<BallState> { red, white, blue };
                StabilizationStatus status = controller.stateChanged(perturb(balls, maxPerturbation), timeStep);
                if (lastStatus == StabilizationStatus.STABLE) {
                    // As soon as status has changed to STABLE, it should not change anymore for stability purposes.
                    Assert.AreEqual(StabilizationStatus.STABLE, status);
                }
                lastStatus = status;
            }
            Assert.AreEqual(StabilizationStatus.STABLE, lastStatus);
            Debug.Log("White starts moving, Red is stationary ------------------------------------------------------------");
            for (double i = 0.0; i < whiteToTarget.length(); i += step) {
                List<BallState> balls = new List<BallState> { red, positionedAt(white, white.position + whiteDirection * i), perturb(blue, maxPerturbation) };
                StabilizationStatus status = controller.stateChanged(balls, timeStep);
                if (lastStatus == StabilizationStatus.UNSTABLE) {
                    // As soon as status has changed to UNSTABLE, it should not change anymore for stability purposes.
                    Assert.AreEqual(StabilizationStatus.UNSTABLE, status);
                }
                lastStatus = status;
            }
            Assert.AreEqual(StabilizationStatus.UNSTABLE, lastStatus);
            Debug.Log("White is stationary, Red starts moving ------------------------------------------------------------");
            for (double i = 0.0; i < redToTarget.length(); i += step) {
                List<BallState> balls = new List<BallState> { positionedAt(red, red.position + redDirection * i), positionedAt(white, whiteTarget), perturb(blue, maxPerturbation) };
                StabilizationStatus status = controller.stateChanged(balls, timeStep);
                Assert.AreEqual(StabilizationStatus.UNSTABLE, status);
                lastStatus = status;
            }
            Assert.AreEqual(StabilizationStatus.UNSTABLE, lastStatus);
            Debug.Log("No ball moving anymore ------------------------------------------------------------");
            for (int i = 0; i < 2.0f / timeStep + 5; i++) {
                List<BallState> balls = new List<BallState> { positionedAt(red, redTarget), positionedAt(white, whiteTarget), blue };
                StabilizationStatus status = controller.stateChanged(perturb(balls, maxPerturbation), timeStep);
                if (lastStatus == StabilizationStatus.STABLE) {
                    // As soon as status has changed to STABLE, it should not change anymore for stability purposes.
                    Assert.AreEqual(StabilizationStatus.STABLE, status);
                }
                lastStatus = status;
            }
            Assert.AreEqual(StabilizationStatus.STABLE, lastStatus);
        }

        [Test]
        public void stateChanged_should_handle_cue_ball_disappearing_and_reappearing() {

            StabilizationController controller = new StabilizationController();
            controller.track = true;

            // Note that the red ball should roll further than the white one
            BallState white = new BallState { type = "WHITE", id = "WHITE1", position = new Vec2 { x = 0, y = 0} };
            Vec2 whiteTarget = new Vec2 { x = 940, y = 440};
            Vec2 whiteToTarget = whiteTarget - white.position;
            Vec2 whiteDirection = whiteToTarget.normalized();
            BallState red = new BallState { type = "RED", id = "RED1", position = new Vec2 { x = 200, y = 0} };
            BallState blue = new BallState { type = "BLUE", id = "BLUE1", position = new Vec2 { x = -400, y = -250} };

            float timeStep = 0.1f;
            double step = 50.0; // In Millimeters
            float maxPerturbation = 3; // in millimeters (in both directions)

            Debug.Log("No ball moving yet ------------------------------------------------------------");
            StabilizationStatus lastStatus = StabilizationStatus.UNKNOWN;
            controller.stateChanged(new List<BallState> { red, white, blue }, 0.0f);
            for (int i = 0; i < 2.0f / timeStep + 5; i++) {
                List<BallState> balls = new List<BallState> { red, white, blue };
                StabilizationStatus status = controller.stateChanged(perturb(balls, maxPerturbation), timeStep);
                if (lastStatus == StabilizationStatus.STABLE) {
                    // As soon as status has changed to STABLE, it should not change anymore for stability purposes.
                    Assert.AreEqual(StabilizationStatus.STABLE, status);
                }
                lastStatus = status;
            }
            Assert.AreEqual(StabilizationStatus.STABLE, lastStatus);
            Debug.Log("White starts moving, Others are stationary ------------------------------------------------------------");
            for (double i = 0.0; i < whiteToTarget.length(); i += step) {
                List<BallState> balls = new List<BallState> { positionedAt(white, white.position + whiteDirection * i), perturb(red, maxPerturbation), perturb(blue, maxPerturbation) };
                StabilizationStatus status = controller.stateChanged(balls, timeStep);
                if (lastStatus == StabilizationStatus.UNSTABLE) {
                    // As soon as status has changed to UNSTABLE, it should not change anymore for stability purposes.
                    Assert.AreEqual(StabilizationStatus.UNSTABLE, status);
                }
                lastStatus = status;
            }
            Assert.AreEqual(StabilizationStatus.UNSTABLE, lastStatus);
            Debug.Log("White fell into pocket ------------------------------------------------------------");
            for (int i = 0; i < 2.0f / timeStep + 5; i++) {
                List<BallState> balls = new List<BallState> { perturb(red, maxPerturbation), perturb(blue, maxPerturbation) };
                StabilizationStatus status = controller.stateChanged(balls, timeStep);
                if (lastStatus == StabilizationStatus.UNSTABLE) {
                    // As soon as status has changed to UNSTABLE, it should not change anymore for stability purposes.
                    Assert.AreEqual(StabilizationStatus.UNSTABLE, status);
                }
                lastStatus = status;
            }
            Assert.AreEqual(StabilizationStatus.UNSTABLE, lastStatus);
            Debug.Log("White was put on table ------------------------------------------------------------");
            for (int i = 0; i < 2.0f / timeStep + 5; i++) {
                List<BallState> balls = new List<BallState> { red, blue, white };
                StabilizationStatus status = controller.stateChanged(perturb(balls, maxPerturbation), timeStep);
                if (lastStatus == StabilizationStatus.STABLE) {
                    // As soon as status has changed to STABLE, it should not change anymore for stability purposes.
                    Assert.AreEqual(StabilizationStatus.STABLE, status);
                }
                lastStatus = status;
            }
            Assert.AreEqual(StabilizationStatus.STABLE, lastStatus);
        }

        [Test]
        public void stateChanged_should_handle_cue_ball_rolling_slowly_for_a_short_time() {

            StabilizationController controller = new StabilizationController();
            controller.track = true;

            // Note that the red ball should roll further than the white one
            BallState white = new BallState { type = "WHITE", id = "WHITE1", position = new Vec2 { x = 0, y = 0} };
            Vec2 whiteTarget = new Vec2 { x = 50, y = 0};
            Vec2 whiteToTarget = whiteTarget - white.position;
            Vec2 whiteDirection = whiteToTarget.normalized();

            float timeStep = 0.1f;
            double step = 5.0; // In Millimeters
            float maxPerturbation = 3; // in millimeters (in both directions)

            Debug.Log("White not moving yet ------------------------------------------------------------");
            StabilizationStatus lastStatus = StabilizationStatus.UNKNOWN;
            controller.stateChanged(new List<BallState> { white }, 0.0f);
            for (int i = 0; i < 2.0f / timeStep + 5; i++) {
                List<BallState> balls = new List<BallState> { white };
                StabilizationStatus status = controller.stateChanged(perturb(balls, maxPerturbation), timeStep);
                if (lastStatus == StabilizationStatus.STABLE) {
                    // As soon as status has changed to STABLE, it should not change anymore for stability purposes.
                    Assert.AreEqual(StabilizationStatus.STABLE, status);
                }
                lastStatus = status;
            }
            Assert.AreEqual(StabilizationStatus.STABLE, lastStatus);
            Debug.Log("White starts moving ------------------------------------------------------------");
            for (double i = 0.0; i < whiteToTarget.length(); i += step) {
                List<BallState> balls = new List<BallState> { positionedAt(white, white.position + whiteDirection * i) };
                StabilizationStatus status = controller.stateChanged(balls, timeStep);
                if (lastStatus == StabilizationStatus.UNSTABLE) {
                    // As soon as status has changed to UNSTABLE, it should not change anymore for stability purposes.
                    Assert.AreEqual(StabilizationStatus.UNSTABLE, status);
                }
                lastStatus = status;
            }
            Assert.AreEqual(StabilizationStatus.UNSTABLE, lastStatus);
            Debug.Log("White stopped moving ------------------------------------------------------------");
            for (int i = 0; i < 2.0f / timeStep + 5; i++) {
                List<BallState> balls = new List<BallState> { white };
                StabilizationStatus status = controller.stateChanged(perturb(balls, maxPerturbation), timeStep);
                if (lastStatus == StabilizationStatus.STABLE) {
                    // As soon as status has changed to STABLE, it should not change anymore for stability purposes.
                    Assert.AreEqual(StabilizationStatus.STABLE, status);
                }
                lastStatus = status;
            }
            Assert.AreEqual(StabilizationStatus.STABLE, lastStatus);
        }

        [Test]
        public void stateChanged_should_handle_colored_ball_disappearing_and_reappearing() {

            StabilizationController controller = new StabilizationController();
            controller.track = true;

            // Note that the red ball should roll further than the white one
            BallState white = new BallState { type = "WHITE", id = "WHITE1", position = new Vec2 { x = 0, y = 0} };
            BallState red = new BallState { type = "RED", id = "RED1", position = new Vec2 { x = 200, y = 0} };
            BallState blue = new BallState { type = "BLUE", id = "BLUE1", position = new Vec2 { x = -400, y = -250} };
            BallState blue2 = new BallState { type = "BLUE", id = "BLUE2", position = new Vec2 { x = 100, y = 95} };
            Vec2 blueTarget = new Vec2 { x = -940, y = -440};
            Vec2 blueToTarget = blueTarget - blue.position;
            Vec2 blueDirection = blueToTarget.normalized();

            float timeStep = 0.1f;
            double step = 50.0; // In Millimeters
            float maxPerturbation = 3; // in millimeters (in both directions)

            Debug.Log("No ball moving yet ------------------------------------------------------------");
            StabilizationStatus lastStatus = StabilizationStatus.UNKNOWN;
            controller.stateChanged(new List<BallState> { red, white, blue }, 0.0f);
            for (int i = 0; i < 2.0f / timeStep + 5; i++) {
                List<BallState> balls = new List<BallState> { red, white, blue };
                StabilizationStatus status = controller.stateChanged(perturb(balls, maxPerturbation), timeStep);
                if (lastStatus == StabilizationStatus.STABLE) {
                    // As soon as status has changed to STABLE, it should not change anymore for stability purposes.
                    Assert.AreEqual(StabilizationStatus.STABLE, status);
                }
                lastStatus = status;
            }
            Assert.AreEqual(StabilizationStatus.STABLE, lastStatus);
            Debug.Log("Blue starts moving, Others are stationary ------------------------------------------------------------");
            for (double i = 0.0; i < blueToTarget.length(); i += step) {
                List<BallState> balls = new List<BallState> { positionedAt(blue, blue.position + blueDirection * i), perturb(red, maxPerturbation), perturb(white, maxPerturbation) };
                StabilizationStatus status = controller.stateChanged(balls, timeStep);
                if (lastStatus == StabilizationStatus.UNSTABLE) {
                    // As soon as status has changed to UNSTABLE, it should not change anymore for stability purposes.
                    Assert.AreEqual(StabilizationStatus.UNSTABLE, status);
                }
                lastStatus = status;
            }
            Assert.AreEqual(StabilizationStatus.UNSTABLE, lastStatus);
            Debug.Log("Blue fell into pocket ------------------------------------------------------------");
            for (int i = 0; i < 2.0f / timeStep + 5; i++) {
                List<BallState> balls = new List<BallState> { perturb(red, maxPerturbation), perturb(white, maxPerturbation) };
                StabilizationStatus status = controller.stateChanged(balls, timeStep);
                if (lastStatus == StabilizationStatus.STABLE) {
                    // As soon as status has changed to STABLE, it should not change anymore for stability purposes.
                    Assert.AreEqual(StabilizationStatus.STABLE, status);
                }
                lastStatus = status;
            }
            Assert.AreEqual(StabilizationStatus.STABLE, lastStatus);
            Debug.Log("Blue was put on table ------------------------------------------------------------");
            lastStatus = controller.stateChanged(perturb(new List<BallState> { red, blue2, white }, maxPerturbation), timeStep);
            Assert.AreEqual(StabilizationStatus.UNSTABLE, lastStatus);

            for (int i = 0; i < 2.0f / timeStep + 5; i++) {
                List<BallState> balls = new List<BallState> { red, blue2, white };
                StabilizationStatus status = controller.stateChanged(perturb(balls, maxPerturbation), timeStep);
                if (lastStatus == StabilizationStatus.STABLE) {
                    // As soon as status has changed to STABLE, it should not change anymore for stability purposes.
                    Assert.AreEqual(StabilizationStatus.STABLE, status);
                }
                lastStatus = status;
            }
            Assert.AreEqual(StabilizationStatus.STABLE, lastStatus);
        }

        [Test]
        public void findNearestBall_should_return_nearest() {

            StabilizationController controller = new StabilizationController();
            controller.track = true;

            Vec2 position = new Vec2 { x = 100, y = 50};
            BallState whiteBall = new BallState { type = "WHITE", id = "WHITE1", position = new Vec2 { x = 0, y = 0} };

            List<BallState> balls = new List<BallState> {
                new BallState { type = "RED", id = "RED1", position = new Vec2 { x = 400, y = 300} },
                whiteBall
            };

            var (nearest, squaredDistance) = controller.findNearestBall(balls, position);
            Assert.AreEqual(whiteBall, nearest);
        }

        private List<BallState> perturb(List<BallState> balls, float max) {
            List<BallState> result = new List<BallState>();

            var random = new System.Random(DateTime.Now.Millisecond);
            foreach (BallState ball in balls) {
                float x = (float)(ball.position.x + (2.0 * random.NextDouble() - 1.0) * max);
                float y = (float)(ball.position.y + (2.0 * random.NextDouble() - 1.0) * max);
                result.Add(positionedAt(ball, new Vec2 {x = x, y = y}));
            }

            return result;
        }

        private BallState perturb(BallState ball, float max) {
            var random = new System.Random(DateTime.Now.Millisecond);
            float x = (float)(ball.position.x + (2.0 * random.NextDouble() - 1.0) * max);
            float y = (float)(ball.position.y + (2.0 * random.NextDouble() - 1.0) * max);
            return positionedAt(ball, new Vec2 {x = x, y = y});
        }

        private BallState positionedAt(BallState ball, Vec2 position) {
            return new BallState { id = ball.id, type = ball.type, position = position };
        }
    }
}
