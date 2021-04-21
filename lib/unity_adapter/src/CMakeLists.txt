target_sources(${PROJECT_NAME}
        PUBLIC
        ../include/unity_adapter/unity_adapter.hpp
        ../include/unity_adapter/type.hpp

        PRIVATE
        event_queue.hpp
        unity_adapter.cpp
        type.cpp)
