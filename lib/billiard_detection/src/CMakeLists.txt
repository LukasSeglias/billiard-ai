target_sources(${PROJECT_NAME}
        PUBLIC
        ../include/billiard_detection/billiard_detection.hpp
        ../include/billiard_detection/type.hpp

        PRIVATE
        billiard_detection.cpp
        type.cpp)
