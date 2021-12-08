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
}

class BilliardSearchTest :public ::testing::TestWithParam<std::string> {
};

void showResults(const billiard::search::State& state, const billiard::search::Search& search,
                 int solutions, const billiard::search::Configuration& config) {

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

TEST(BilliardSearchTest, three_balls_in_a_row_with_cueball_and_pocket_manually_set) {

    float ballDiameter = 52.3;
    glm::vec2 pocketNormal { 0.707107, 0.707107};

    billiard::search::State state { {
            billiard::search::Ball { glm::vec2 {-739.289, -241.31}, "WHITE", "WHITE-1" },
            billiard::search::Ball { glm::vec2 {-772.974, -292.356}, "RED", "RED-3" },
            billiard::search::Ball { glm::vec2 {-810.029, -336.596}, "RED", "RED-2" },
            billiard::search::Ball { glm::vec2 {-843.714, -384.238}, "RED", "RED-1" },
    } };
    billiard::search::Search search { "RED-1", {} };
    uint16_t solutions = 10;
    billiard::search::Configuration config {};
    config._ball._radius = ballDiameter / 2.0f;
    config._ball._diameter = ballDiameter;
    config._ball._diameterSquared = ballDiameter * ballDiameter;
    config._table._pockets = {
            billiard::search::Pocket { "BOTTOM-LEFT", billiard::search::PocketType::CORNER, glm::vec2 {-915.5, -456.5}, pocketNormal, 52}
    };
    config._table.minimalPocketVelocity = 10.0f;

    showResults(state, search, solutions, config);
}

TEST(BilliardSearchTest, three_balls_in_a_row_with_cueball_and_pocket) {

    float ballDiameter = 52.3;
    float space = 0.1;
    glm::vec2 whitePosition {0, 0};
    glm::vec2 pocketPosition { -700, -400};
    glm::vec2 pocketNormal { 0.707107, 0.707107};
    glm::vec2 whiteToPocket = pocketPosition - whitePosition;
    glm::vec2 whiteToPocketDirection = glm::normalize(whiteToPocket);
    glm::vec2 red3 = whitePosition + (ballDiameter + space) * whiteToPocketDirection;
    glm::vec2 red2 = red3 + (ballDiameter + space) * whiteToPocketDirection;
    glm::vec2 red1 = red2 + (ballDiameter + space) * whiteToPocketDirection;

    billiard::search::State state { {
        billiard::search::Ball { whitePosition, "WHITE", "WHITE-1" },
        billiard::search::Ball { red3, "RED", "RED-3" },
        billiard::search::Ball { red2, "RED", "RED-2" },
        billiard::search::Ball { red1, "RED", "RED-1" },
    } };

    billiard::search::Search search { "RED-1", {""} };
    uint16_t solutions = 10;
    billiard::search::Configuration config {};
    config._ball._radius = ballDiameter / 2.0f;
    config._ball._diameter = ballDiameter;
    config._ball._diameterSquared = ballDiameter * ballDiameter;
    config._table._pockets = {
            billiard::search::Pocket { "BOTTOM-LEFT", billiard::search::PocketType::CORNER, pocketPosition, pocketNormal, 52}
    };
    config._table.minimalPocketVelocity = 10.0f;

    showResults(state, search, solutions, config);
}

TEST(BilliardSearchTest, one_ball_and_cue_ball_and_pocket) {

    float ballDiameter = 52.3;
    glm::vec2 whitePosition {0, 0};
    glm::vec2 red1 {-650, 0};

    billiard::search::State state { {
                                            billiard::search::Ball { whitePosition, "WHITE", "WHITE-1" },
                                            billiard::search::Ball { red1, "RED", "RED-1" },
                                    } };

    billiard::search::Search search { "RED-1", {""} };
    uint16_t solutions = 1000;
    billiard::search::Configuration config {};
    config._ball._radius = ballDiameter / 2.0f;
    config._ball._diameter = ballDiameter;
    config._ball._diameterSquared = ballDiameter * ballDiameter;
    config._table._pockets = {
            billiard::search::Pocket { "TOP RIGHT", billiard::search::PocketType::CORNER, glm::vec2{940.5, 471.5}, glm::vec2{-0.707107, -0.707107}, 50},
            billiard::search::Pocket { "TOP LEFT", billiard::search::PocketType::CORNER, glm::vec2{-940.5, 471.5}, glm::vec2{0.707107, -0.707107}, 50},
            billiard::search::Pocket { "TOP CENTER", billiard::search::PocketType::CENTER, glm::vec2{0, 491.5}, glm::vec2{0, -1}, 50},
            billiard::search::Pocket { "BOTTOM RIGHT", billiard::search::PocketType::CORNER, glm::vec2{940.5, -471.5}, glm::vec2{-0.707107, 0.707107}, 50},
            billiard::search::Pocket { "BOTTOM LEFT", billiard::search::PocketType::CORNER, glm::vec2{-940.5, -471.5}, glm::vec2{0.707107, 0.707107}, 50},
            billiard::search::Pocket { "BOTTOM CENTER", billiard::search::PocketType::CENTER, glm::vec2{0, -491.5}, glm::vec2{0, 1}, 50}
    };

    config._table.minimalPocketVelocity = 10.0f;
    auto innerTableLength = 1881;
    auto innerTableWidth = 943;
    config._table.diagonalLengthSquared = innerTableLength * innerTableLength + innerTableWidth * innerTableWidth;
    config._table._rails = {
            {"Left", billiard::search::Rail{"Left", glm::vec2{-940.5, 390}, glm::vec2{-940.5, -390}, glm::vec2{1, 0}, config._ball._radius} },
            {"Right", billiard::search::Rail{"Right", glm::vec2{940.5, -390}, glm::vec2{940.5, 390}, glm::vec2{-1, 0}, config._ball._radius} },
            {"Bottom Left", billiard::search::Rail{"Bottom Left", glm::vec2{-862.5, -471.5}, glm::vec2{-57.5, -471.5}, glm::vec2{0, 1}, config._ball._radius} },
            {"Bottom Right", billiard::search::Rail{"Bottom Right", glm::vec2{57.5, -471.5}, glm::vec2{862.5, -471.5}, glm::vec2{0, 1}, config._ball._radius} },
            {"Top Left", billiard::search::Rail{"Top Left", glm::vec2{-55, 471.5}, glm::vec2{-860, 471.5}, glm::vec2{0, -1}, config._ball._radius} },
            {"Top Right", billiard::search::Rail{"Top Right", glm::vec2{860, 471.5}, glm::vec2{55, 471.5}, glm::vec2{0, -1}, config._ball._radius} }
    };

    showResults(state, search, solutions, config);
}

TEST(BilliardSearchTest, visualizationOfManualCase) {

    billiard::search::State state { {} };
    billiard::search::Search search { "", {"RED"} };
    uint16_t solutions = 10;
    billiard::search::Configuration config {};

    showResults(state, search, solutions, config);
}

TEST(BilliardSearchTest, visualizationOfTestCase) {

    billiard::search::Configuration config = loadConfig("./resources/configuration.json");


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

    billiard::search::Configuration config = loadConfig("./resources/configuration.json");

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
