#include <gtest/gtest.h>
#include <billiard_search/billiard_search.hpp>
#include <billiard_snooker/billiard_snooker.hpp>
#include <fstream>
#include <nlohmann/json.hpp>
#include "billiard_search_visualization.hpp"
#include "util.hpp"

#define SOLUTIONS 10

namespace billiard::search::test {
    void printDescription(nlohmann::json& json);
    billiard::search::State state(nlohmann::json& json);
    billiard::search::Search search(nlohmann::json& json);
    billiard::search::Configuration config(nlohmann::json& json);
    billiard::search::Configuration loadConfig(const std::string& configFilepath);
}

class BilliardSearchTest :public ::testing::TestWithParam<std::string> {
};

TEST(BilliardSearchTest, visualizationOfManualCase) {

    billiard::search::State state { {} };
    billiard::search::Search search { "", "RED" };
    uint16_t solutions = 10;
    billiard::search::Configuration config {};

    auto future = billiard::search::searchOnly(state, search, solutions, config);

    future.wait();
    auto results = future.get();

    int resultIndex = 0;
    bool resultChanged = true;
    while (true) {

        if (resultChanged) {

            std::shared_ptr<billiard::search::SearchNode> node;
            if (results.empty()) {
                node = nullptr;
            } else {
                node = results[resultIndex];
            }

            cv::Mat searchResultImage = visualize(state, node);
            cv::imshow("Result", searchResultImage);

            resultChanged = false;
        }

        char key = (char) cv::waitKey(30);
        if (key == 'q' || key == 27) {
            break;
        } else if (key == 's') {
            std::cout << "Search" << std::endl;
            auto future = billiard::search::searchOnly(state, search, solutions, config);
            future.wait();
            results = future.get();
            resultIndex = 0;
            resultChanged = true;
        } else if (key == 'l') {
            resultChanged = true;
        } else if (key == 97 /* A */) {
            int lastIndex = results.empty() ? 0 : results.size() - 1;
            resultIndex = resultIndex == 0 ? lastIndex : resultIndex - 1;
            resultChanged = true;
        } else if (key == 100 /* D */) {
            resultIndex = results.empty() ? 0 : (resultIndex + 1) % (results.size());
            resultChanged = true;
        }
    }
}

TEST(BilliardSearchTest, visualizationOfTestCase) {

    billiard::search::Configuration config = billiard::search::test::loadConfig("./resources/configuration.json");


    const auto& descriptionPath = "./resources/00_test_01.json";
    std::ifstream descriptionFile{descriptionPath};
    nlohmann::json descriptionJson;
    descriptionFile >> descriptionJson;

    auto test = descriptionJson;
    std::cout << "TEST: -- " << test["test"] << std::endl;
    billiard::search::test::printDescription(test["description"]);

    billiard::search::State state = billiard::search::test::state(test["state"]);
    billiard::search::Search search = billiard::search::test::search(test["search"]);

    uint64_t tick = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

    auto future = billiard::search::searchOnly(state, search, SOLUTIONS, config);
    future.wait();

    uint64_t tack = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    uint64_t time = tack - tick;
    std::cout << "Computation time [ms]: " << time << std::endl;
    uint64_t timeSum = time;
    uint32_t executions = 1;

    auto results = future.get();
    std::cout << "Results: " << std::to_string(results.size()) << std::endl;

    int resultIndex = 0;
    bool resultChanged = true;
    while (true) {

        if (resultChanged) {

            std::shared_ptr<billiard::search::SearchNode> node;
            if (results.empty()) {
                node = nullptr;
            } else {
                node = results[resultIndex];
            }

            cv::Mat searchResultImage = visualize(state, node);
            cv::imshow("Result", searchResultImage);

            resultChanged = false;
        }

        char key = (char) cv::waitKey(30);
        if (key == 'q' || key == 27) {
            break;
        } else if (key == 's') {
            std::cout << "Search" << std::endl;
            tick = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count();
            auto future = billiard::search::searchOnly(state, search, SOLUTIONS, config);
            future.wait();
            tack = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count();
            time = tack - tick;
            timeSum += time;
            executions++;
            std::cout << "Computation time [ms]: " << time << std::endl;
            std::cout << "Average computation time [ms]: " << (timeSum / executions) << std::endl;
            results = future.get();
            std::cout << "Results: " << std::to_string(results.size()) << std::endl;
            resultIndex = 0;
            resultChanged = true;
        } else if (key == 'l') {
            resultChanged = true;
        } else if (key == 97 /* A */) {
            int lastIndex = results.empty() ? 0 : results.size() - 1;
            resultIndex = resultIndex == 0 ? lastIndex : resultIndex - 1;
            resultChanged = true;
        } else if (key == 100 /* D */) {
            resultIndex = results.empty() ? 0 : (resultIndex + 1) % (results.size());
            resultChanged = true;
        }
    }
}

TEST(BilliardSearchTest, visualizationWithDummyData) {

    using billiard::search::SearchNode;
    using billiard::search::Ball;
    using billiard::search::State;
    using billiard::search::SearchNodeType;
    using billiard::search::SearchActionType;
    using billiard::search::SearchNodeSearch;
    using billiard::search::PhysicalEvent;
    using billiard::search::PhysicalEventType;

    {
        State state ({
                             Ball { glm::vec2 {0,0}, "RED", "RED-1" },
                             Ball { glm::vec2 {700,-34}, "BLACK", "BLACK-1" },
                             Ball { glm::vec2 {120,400}, "WHITE", "WHITE-1" },
                             Ball { glm::vec2 {-876,205}, "YELLOW", "YELLOW-1" },
                             Ball { glm::vec2 {-765,-100}, "BLUE", "BLUE-1" },
                     });

        cv::Mat stateImage = visualize(state, nullptr);
        cv::imshow("Test 1", stateImage);
    }

    {
        State state ({
                             Ball { glm::vec2 {870,-400}, "RED", "RED-1" },
                             Ball { glm::vec2 {150,-100}, "RED", "RED-1" },
                             Ball { glm::vec2 {-200,100}, "WHITE", "WHITE-1" }
                     });

        std::shared_ptr<SearchNode> pocket = createNode(SearchActionType::NONE, "", {}, nullptr);
        std::shared_ptr<SearchNode> red = createNode(SearchActionType::DIRECT, "RED-1", {
                PhysicalEvent { PhysicalEventType::POCKET_COLLISION, glm::vec2 {900, -440} }
        }, pocket);
        std::shared_ptr<SearchNode> lightRed = createNode(SearchActionType::DIRECT, "RED-2", {
                PhysicalEvent { PhysicalEventType::BALL_COLLISION, glm::vec2 {860, -390} }
        }, red);
        std::shared_ptr<SearchNode> white = createNode(SearchActionType::RAIL, "WHITE-1", {
                PhysicalEvent { PhysicalEventType::BALL_COLLISION, glm::vec2 {860, -390} },
                PhysicalEvent { PhysicalEventType::BALL_COLLISION, glm::vec2 {0, 440} },
                PhysicalEvent { PhysicalEventType::BALL_KICK, glm::vec2 {-200,100} }
        }, red);

        cv::Mat searchResultImage = visualize(state, white);
        cv::imshow("Test 2", searchResultImage);
    }
    cv::waitKey();
}

TEST_P(BilliardSearchTest, doSearch) {

    billiard::search::Configuration config = billiard::search::test::loadConfig("./resources/configuration.json");

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
                                           config);

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

billiard::search::Configuration billiard::search::test::loadConfig(const std::string& configFilepath) {
    std::ifstream configFile{configFilepath};
    nlohmann::json json;
    configFile >> json;
    return billiard::search::test::config(json);
}

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
    float ballRadius = json["ball"]["radius"];
    std::vector<Rail> rails;
    for (auto& rail : json["banks"]) {
        rails.emplace_back(Rail{rail["id"],
                                glm::vec2{rail["start"]["x"], rail["start"]["y"]},
                                glm::vec2{rail["end"]["x"], rail["end"]["y"]},
                                ballRadius,
                                rail["location"]});
    }
    auto conf = Configuration{};
    conf._ball._radius = ballRadius;
    conf._ball._diameterSquared = (conf._ball._radius * 2) * (conf._ball._radius * 2);
    conf._table._pockets = pockets;
    conf._table._rails = rails;
    conf._rules._modifyState = billiard::snooker::stateAfterBreak;
    conf._rules._nextTypeToSearch = billiard::snooker::nextSearchType;
    conf._rules._isValidEndState = billiard::snooker::validEndState;

    return conf;
}