#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>

#include "macro_definition.hpp"

namespace billiard::detection {

    struct EXPORT_BILLIARD_DETECTION_LIB Ball {
        bool operator==(const Ball& other) const;

        glm::vec2 _position;
        std::string _type;
        std::string _id;
    };

    struct EXPORT_BILLIARD_DETECTION_LIB State {
        bool operator==(const State& other) const;

        std::vector<Ball> _balls;
    };

    struct EXPORT_BILLIARD_DETECTION_LIB Pocket {
        double x;
        double y;
        double radius;
        Pocket(double x, double y, double radius): x(x), y(y), radius(radius) {};
    };

    struct EXPORT_BILLIARD_DETECTION_LIB RailSegment {
        glm::vec2 start;
        glm::vec2 end;
    };

}