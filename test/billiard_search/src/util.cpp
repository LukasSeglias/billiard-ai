#include "util.hpp"

std::shared_ptr<billiard::search::SearchNode> createNode(billiard::search::SearchActionType actionType,
                                                         const std::string& ballId,
                                                         const std::vector<billiard::search::PhysicalEvent>&& events,
                                                         const std::shared_ptr<billiard::search::SearchNode>& parent) {
    auto searchNode = billiard::search::SearchNode::search(parent);
    std::shared_ptr<billiard::search::SearchNodeSearch> test = searchNode->asSearch();
    test->_action = actionType;
    test->_ballId = ballId;
    test->_events = events;
    return searchNode;
}