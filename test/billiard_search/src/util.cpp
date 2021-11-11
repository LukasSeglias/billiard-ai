#include "util.hpp"
#include <fstream>
#include <billiard_snooker/billiard_snooker.hpp>

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

billiard::search::Configuration loadConfig(const std::string& configFilepath) {
    std::ifstream configFile{configFilepath};
    nlohmann::json json;
    configFile >> json;
    return config(json);
}

billiard::search::Configuration config(nlohmann::json& json) {

    using billiard::search::PocketType;
    using billiard::search::Pocket;
    using billiard::search::Rail;
    using billiard::search::Configuration;

    static const std::unordered_map<std::string, PocketType> pocketTypes = {
            {"CORNER", PocketType::CORNER},
            {"CENTER", PocketType::CENTER}
    };

    std::vector<Pocket> pockets;
    for (auto& pocket : json["pockets"]) {
        pockets.emplace_back(Pocket{pocket["id"],
                                    pocket["type"],
                                    glm::vec2{pocket["position"]["x"],
                                              pocket["position"]["y"]},
                                    glm::vec2{pocket["normal"]["x"],
                                              pocket["normal"]["y"]},
                                    pocket["radius"]});
    }
    float ballRadius = json["ball"]["radius"];
    std::vector<Rail> rails;
    for (auto& rail : json["banks"]) {
        rails.emplace_back(Rail{rail["id"],
                                glm::vec2{rail["start"]["x"], rail["start"]["y"]},
                                glm::vec2{rail["end"]["x"], rail["end"]["y"]},
                                ballRadius});
    }
    auto conf = Configuration{};
    conf._ball._radius = ballRadius;
    conf._ball._diameter = ballRadius * 2;
    conf._ball._diameterSquared = (conf._ball._radius * 2) * (conf._ball._radius * 2);
    conf._table._pockets = pockets;
    conf._table._rails = rails;
    float innerTableLength = json["table"]["innerTableLength"];
    float innerTableWidth = json["table"]["innerTableWidth"];
    conf._table.diagonalLengthSquared = innerTableLength * innerTableLength + innerTableWidth * innerTableWidth;
    conf._rules._modifyState = billiard::snooker::stateAfterBreak;
    conf._rules._nextSearch = billiard::snooker::nextSearch;
    conf._rules._isValidEndState = billiard::snooker::validEndState;
    conf._rules._scoreForPottedBall = billiard::snooker::scoreForPottedBall;

    return conf;
}
