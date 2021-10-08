#pragma once

#include "macro_definition.hpp"
#include <glm/glm.hpp>
#include <vector>

extern "C" {

    struct EXPORT_UNITY_ADAPTER_LIB Vec2 {
        double x;
        double y;
    };

    struct EXPORT_UNITY_ADAPTER_LIB Vec3 {
        double x;
        double y;
        double z;
    };

    enum RailLocation {
        TOP,
        BOTTOM,
        RIGHT,
        LEFT
    };

    struct EXPORT_UNITY_ADAPTER_LIB RailSegment {
        Vec2 start;
        Vec2 end;
        RailLocation location;
    };

    struct EXPORT_UNITY_ADAPTER_LIB Circle {
        float radius;
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
        int segmentSize;
        int targetSize;
        RailSegment* segments;
        Circle* targets;
        ArucoMarkers markers;
        CameraIntrinsics camera;
        Plane ballPlane;
        WorldToModel worldToModel;
        Table table;
    };

    struct EXPORT_UNITY_ADAPTER_LIB  Ball {
        Ball();
        Ball(const char* type, const char* id, Vec2 position, Vec2 velocity, bool visible);
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
        BallState(const char* type, const char* id, Vec2 position, bool fromUnity = true);
        BallState(BallState&& other) noexcept;
        BallState(const BallState& other) noexcept;
        BallState& operator=(BallState&& other) noexcept;
        BallState& operator=(const BallState& other) noexcept;
        ~BallState();

        char* type;
        char* id;
        Vec2 position;
        bool fromUnity;
    };

    struct EXPORT_UNITY_ADAPTER_LIB State {
        State();
        State(BallState* balls, int ballSize, bool fromUnity = true);
        State(State&& other) noexcept;
        State(const State& other) noexcept;
        State& operator=(State&& other) noexcept;
        State& operator=(const State& other) noexcept;
        ~State();

        BallState* balls;
        int ballSize;
        bool fromUnity;
    };

    struct EXPORT_UNITY_ADAPTER_LIB Search {
        char* id;
        char* type;
    };

    typedef void (__stdcall* AnimationChangedEventCallback)(RootObject);
    typedef void (__stdcall* StateChangedEventCallback)(State);
    typedef void (__stdcall* Debugger)(const char*);
}
