#pragma once

#include <librealsense2/hpp/rs_pipeline.hpp.>
#include <utility>

namespace billiard::capture {

    class Camera {
    public:
        rs2::pipeline pipe {};
    };

}
