target_sources(${PROJECT_NAME}
        PUBLIC
        ../include/billiard_capture/billiard_capture.hpp

        PRIVATE
        billiard_capture.cpp
        image_stream.hpp
        image_stream.cpp
        )
