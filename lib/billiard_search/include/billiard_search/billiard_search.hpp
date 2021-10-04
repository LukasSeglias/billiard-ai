#pragma once

#include "macro_definition.hpp"
#include "type.hpp"
#include <future>

namespace billiard::search {

    std::future<std::vector<std::vector<node::System>>> EXPORT_BILLIARD_SEARCH_LIB
    search(const State& state, const Search& search, uint16_t solutions, const Configuration& config);
}