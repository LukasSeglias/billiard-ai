#pragma once

#include <billiard_search/billiard_search.hpp>

std::shared_ptr<billiard::search::SearchNode> createNode(billiard::search::SearchActionType actionType,
                                                   const std::string& ballId,
                                                   const std::vector<billiard::search::PhysicalEvent>&& events,
                                                   const std::shared_ptr<billiard::search::SearchNode>& parent);
