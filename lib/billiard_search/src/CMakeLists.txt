target_sources(${PROJECT_NAME}
        PUBLIC
        ../include/billiard_search/billiard_search.hpp
        ../include/billiard_search/type.hpp

        PRIVATE
        billiard_search.cpp
        type.cpp)
