#include <gtest/gtest.h>
#include <billiard_search/billiard_search.hpp>
#include <billiard_snooker/billiard_snooker.hpp>
#include <fstream>
#include <nlohmann/json.hpp>

#define SOLUTIONS 10

namespace billiard::search::test {
    void printDescription(nlohmann::json& json);
    billiard::search::State state(nlohmann::json& json);
    billiard::search::Search search(nlohmann::json& json);
    billiard::search::Configuration config(nlohmann::json& json);
}

class BilliardSearchTest :public ::testing::TestWithParam<std::string> {
};

TEST_P(BilliardSearchTest, doSearch) {
    const auto& descriptionPath = GetParam();
    std::ifstream descriptionFile{descriptionPath};
    nlohmann::json descriptionJson;
    descriptionFile >> descriptionJson;

    auto test = descriptionJson;
    std::cout << "TEST: -- " << test["test"] << std::endl;
    billiard::search::test::printDescription(test["description"]);

    uint64_t tick = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    auto result = billiard::search::search(billiard::search::test::state(test["state"]),
                                           billiard::search::test::search(test["search"]),
                                           SOLUTIONS,
                                           billiard::search::test::config(test["configuration"]));

    // Assert
    result.wait();
    uint64_t tack = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    uint64_t time = tack - tick;
    std::cout << "Computation time [ms]: " << time << std::endl;

    auto solutions = result.get();
    std::cout << "Solutions: " << solutions.size() << std::endl;
}

INSTANTIATE_TEST_CASE_P(
        BilliardSearch,
        BilliardSearchTest,
        ::testing::Values(
                "./resources/00_test_01.json"
        ));

void billiard::search::test::printDescription(nlohmann::json& json) {
    for(auto& line : json) {
        std::cout << line << std::endl;
    }
}

billiard::search::State billiard::search::test::state(nlohmann::json& json) {
    std::vector<Ball> balls;

    for(auto& ball : json["balls"]) {
        balls.emplace_back(Ball{glm::vec2{ball["position"]["x"], ball["position"]["y"]}, ball["type"], ball["id"]});
    }

    return State{balls};
}

billiard::search::Search billiard::search::test::search(nlohmann::json& json) {
    return Search {json["id"], json["type"]};
}

billiard::search::Configuration billiard::search::test::config(nlohmann::json& json) {

    static const std::unordered_map<std::string, PocketType> pocketTypes = {
            {"CORNER", PocketType::CORNER},
            {"CENTER", PocketType::CENTER}
    };

    std::vector<Pocket> pockets;
    for (auto& pocket : json["pockets"]) {
        pockets.emplace_back(Pocket{pocket["id"],
                                    pocketTypes.at(pocket["type"]),
                                    glm::vec2{pocket["position"]["x"],
                                              pocket["position"]["y"]},
                                    pocket["radius"]});
    }

    std::vector<Rail> rails;
    for (auto& rail : json["banks"]) {
        rails.emplace_back(Rail{rail["id"],
                                glm::vec2{rail["start"]["x"], rail["start"]["y"]},
                                glm::vec2{rail["end"]["x"], rail["end"]["y"]}});
    }
    auto conf = Configuration{};
    conf._ball._radius = json["ball"]["radius"];
    conf._table._pockets = pockets;
    conf._table._rails = rails;
    conf._rules._modifyState = billiard::snooker::stateAfterBreak;
    conf._rules._nextTypeToSearch = billiard::snooker::nextSearchType;
    conf._rules._isValidEndState = billiard::snooker::validEndState;

    return conf;
}