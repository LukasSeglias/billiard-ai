#pragma once

#include "macro_definition.hpp"
#include "type.hpp"
#include <future>

namespace billiard::search {

    std::future<std::vector<std::vector<node::System>>> EXPORT_BILLIARD_SEARCH_LIB
    search(const State& state, const Search& search, uint16_t solutions, const Configuration& config);

    std::future<std::vector<std::shared_ptr<SearchNode>>> EXPORT_BILLIARD_SEARCH_LIB
    searchOnly(const billiard::search::State& state, const billiard::search::Search& search,
               uint16_t solutions, const Configuration& config);

    glm::vec2 EXPORT_BILLIARD_SEARCH_LIB
    calculateMinimalVelocity(const std::vector<const SearchNode*>& nodes, const std::shared_ptr<SearchState>& state);
}