#pragma once

#include "macro_definition.hpp"
#include <glm/glm.hpp>
#include <vector>

extern "C" {

    struct EXPORT_UNITY_ADAPTER_LIB Vec2 {
        double x;
        double y;
    };

    struct EXPORT_UNITY_ADAPTER_LIB RailSegment {
        Vec2 start;
        Vec2 end;
    };

    struct EXPORT_UNITY_ADAPTER_LIB Circle {
        float radius;
        Vec2 position;
    };

    struct EXPORT_UNITY_ADAPTER_LIB ArucoMarker {
        Vec2 position;
        float sideLength;
    };

    struct EXPORT_UNITY_ADAPTER_LIB Configuration {
        float radius;
        float width;
        float height;
        int segmentSize;
        int targetSize;
        int markerSize;
        RailSegment* segments;
        Circle* targets;
        ArucoMarker* markers;
    };

    struct EXPORT_UNITY_ADAPTER_LIB  Ball {
        Ball();
        Ball(char* type, char* id, Vec2 position, Vec2 velocity, bool visible);
        Ball(Ball&& other) noexcept;
        Ball(const Ball& other) noexcept;
        Ball& operator=(Ball&& other) noexcept;
        Ball& operator=(const Ball& other) noexcept;

        char* type;
        char* id;
        Vec2 position;
        Vec2 velocity;
        bool visible;
    };

    struct EXPORT_UNITY_ADAPTER_LIB KeyFrame {
        KeyFrame();
        KeyFrame(double time, Ball balls[], int ballSize);
        KeyFrame(const KeyFrame& other) noexcept;
        KeyFrame(KeyFrame&& other) noexcept;
        KeyFrame& operator=(KeyFrame&& other) noexcept;
        KeyFrame& operator=(const KeyFrame& other) noexcept;
        ~KeyFrame();

        double time;
        Ball* balls;
        int ballSize;
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
        char* type;
        char* id;
        Vec2 position;
    };

    struct EXPORT_UNITY_ADAPTER_LIB State {
        BallState* balls;
        int ballSize;
    };

    struct EXPORT_UNITY_ADAPTER_LIB Search {
        char* id;
        char* type;
    };

    typedef void (__stdcall* AnimationChangedEventCallback)(RootObject);
    typedef void (__stdcall* Debugger)(const char*);
}
