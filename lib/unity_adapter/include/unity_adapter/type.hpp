#pragma once

#include "macro_definition.hpp"
#include <glm/glm.hpp>
#include <vector>

extern "C" {

    struct EXPORT_UNITY_ADAPTER_LIB Vec2 {
        double x;
        double y;

        bool operator!=(const Vec2& other) const;
    };

    struct EXPORT_UNITY_ADAPTER_LIB Vec3 {
        double x;
        double y;
        double z;
    };

    struct EXPORT_UNITY_ADAPTER_LIB RailSegment {
        char* id;
        Vec2 start;
        Vec2 end;
        Vec2 shiftDirection;
    };

    enum PocketType {
        CORNER,
        CENTER
    };

    struct EXPORT_UNITY_ADAPTER_LIB Circle {
        char* id;
        float radius;
        Vec2 position;
        Vec2 normal;
        PocketType pocketType;
    };

    struct EXPORT_UNITY_ADAPTER_LIB Spot {
        char* type;
        Vec2 position;
    };

    struct EXPORT_UNITY_ADAPTER_LIB ArucoMarkers {
        int patternSize;
        float sideLength;
        Vec3 m0;
        Vec3 m1;
        Vec3 m2;
        Vec3 m3;
    };

    struct EXPORT_UNITY_ADAPTER_LIB Plane {
        Vec3 point;
        Vec3 normal;
    };

    struct EXPORT_UNITY_ADAPTER_LIB WorldToModel {
        Vec3 translation;
    };

    struct EXPORT_UNITY_ADAPTER_LIB Table {
        double innerTableLength;
        double innerTableWidth;
        double ballDiameter;
        double arucoHeightAboveInnerTable;
        double railWorldPointZComponent;
        Vec3 worldToRail;
        float minimalPocketVelocity;
    };

    /**
     * Camera intrinsics
     */
    struct EXPORT_UNITY_ADAPTER_LIB CameraIntrinsics {

        // Focal length in pixel
        double fx;
        double fy;

        // Principal point in pixel
        double cx;
        double cy;

        // Skew
        double skew;

        // Radial distortion coefficients k1, k2, k3
        double k1;
        double k2;
        double k3;

        // Tangential distortion coefficients p1, p2
        double p1;
        double p2;

        // Sensor pixel size in millimeters
        double sx;
        double sy;
    };

    struct EXPORT_UNITY_ADAPTER_LIB Configuration {
        float radius;
        float width;
        float height;
        char* headRail;
        int segmentSize;
        int targetSize;
        int spotSize;
        RailSegment* segments;
        Circle* targets;
        Spot* spots;
        ArucoMarkers markers;
        CameraIntrinsics camera;
        Plane ballPlane;
        WorldToModel worldToModel;
        Table table;
        int solutions;
    };

    enum EventType {
        BALL_MOVING,
        BALL_COLLISION,
        BALL_RAIL_COLLISION,
        BALL_POTTING,
        BALL_SHOT,
        BALL_IN_REST
    };

    struct EXPORT_UNITY_ADAPTER_LIB Event {
        Event();
        Event(EventType eventType, const char* involvedBallId);
        Event(Event&& other) noexcept;
        Event(const Event& other) noexcept;
        Event& operator=(Event&& other) noexcept;
        Event& operator=(const Event& other) noexcept;

        EventType eventType;
        char* involvedBallId;
    };

    struct EXPORT_UNITY_ADAPTER_LIB Ball {
        Ball();
        Ball(const char* type, const char* id, Vec2 position, Vec2 velocity, bool visible, Event events);
        Ball(Ball&& other) noexcept;
        Ball(const Ball& other) noexcept;
        Ball& operator=(Ball&& other) noexcept;
        Ball& operator=(const Ball& other) noexcept;
        ~Ball();

        char* type;
        char* id;
        Vec2 position;
        Vec2 velocity;
        bool visible;
        Event events;
    };

    struct EXPORT_UNITY_ADAPTER_LIB KeyFrame {
        KeyFrame();
        KeyFrame(double time, Ball balls[], int ballSize, bool firstFrame);
        KeyFrame(const KeyFrame& other) noexcept;
        KeyFrame(KeyFrame&& other) noexcept;
        KeyFrame& operator=(KeyFrame&& other) noexcept;
        KeyFrame& operator=(const KeyFrame& other) noexcept;
        ~KeyFrame();

        double time;
        Ball* balls;
        int ballSize;
        bool firstFrame;
    };

    struct EXPORT_UNITY_ADAPTER_LIB AnimationModel {
        AnimationModel();
        AnimationModel(KeyFrame keyFrames[], int keyFrameSize);
        AnimationModel(const AnimationModel& other) noexcept;
        AnimationModel(AnimationModel&& other) noexcept;
        AnimationModel& operator=(AnimationModel&& other) noexcept;
        AnimationModel& operator=(const AnimationModel& other) noexcept;
        ~AnimationModel();

        KeyFrame* keyFrames;
        int keyFrameSize;
    };

    struct EXPORT_UNITY_ADAPTER_LIB RootObject {
        RootObject(AnimationModel* animations, int animationSize);
        RootObject(RootObject&& other) noexcept;
        RootObject(const RootObject& other) noexcept;
        RootObject& operator=(RootObject&& other) noexcept;
        RootObject& operator=(const RootObject& other) noexcept;
        ~RootObject();

        AnimationModel* animations;
        int animationSize;
    };

    struct EXPORT_UNITY_ADAPTER_LIB BallState {
        BallState();
        BallState(const char* type, const char* id, Vec2 position, int trackingCount, bool fromUnity = true);
        BallState(BallState&& other) noexcept;
        BallState(const BallState& other) noexcept;
        BallState& operator=(BallState&& other) noexcept;
        BallState& operator=(const BallState& other) noexcept;
        ~BallState();

        char* type;
        char* id;
        Vec2 position;
        int trackingCount;
        bool fromUnity;
    };

    enum EXPORT_UNITY_ADAPTER_LIB TableStatus {
            UNKNOWN,
            STABLE,
            UNSTABLE
    };

    struct EXPORT_UNITY_ADAPTER_LIB State {
        State();
        State(BallState* balls, int ballSize, TableStatus status, Vec2 velocity, bool fromUnity = true);
        State(State&& other) noexcept;
        State(const State& other) noexcept;
        State& operator=(State&& other) noexcept;
        State& operator=(const State& other) noexcept;
        ~State();

        BallState* balls;
        int ballSize;
        TableStatus status;
        Vec2 velocity;
        bool fromUnity;
    };

    struct EXPORT_UNITY_ADAPTER_LIB Text {
        Text();
        Text(const char* text);
        Text(Text&& other) noexcept;
        Text(const Text& other) noexcept;
        Text& operator=(Text&& other) noexcept;
        Text& operator=(const Text& other) noexcept;
        ~Text();

        char* text;
    };

    struct EXPORT_UNITY_ADAPTER_LIB Search {
        char* id;
        Text* types;
        int typeSize;
    };

    typedef void (__stdcall* AnimationChangedEventCallback)(RootObject);
    typedef void (__stdcall* StateChangedEventCallback)(State);
    typedef void (__stdcall* Debugger)(const char*);
}
