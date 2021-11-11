#pragma once

#include <billiard_search/billiard_search.hpp>
#include <nlohmann/json.hpp>

std::shared_ptr<billiard::search::SearchNode> createNode(billiard::search::SearchActionType actionType,
                                                   const std::string& ballId,
                                                   const std::vector<billiard::search::PhysicalEvent>&& events,
                                                   const std::shared_ptr<billiard::search::SearchNode>& parent);

billiard::search::Configuration config(nlohmann::json& json);

billiard::search::Configuration loadConfig(const std::string& configFilepath);