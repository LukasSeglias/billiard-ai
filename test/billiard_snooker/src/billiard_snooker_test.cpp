#include <gtest/gtest.h>
#include <glm/glm.hpp>
#include <billiard_search/billiard_search.hpp>
#include <billiard_snooker/billiard_snooker.hpp>
#include <algorithm>
#include <vector>

namespace test::util {

    template<class T>
    bool contains(const T& expected, const std::vector<T>& check);

    bool ballIsAtPosition(const std::string& type, const glm::vec2& expectedPosition,
                              const billiard::search::State& state);

    bool ballIsNextToPosition(const std::string& type, const glm::vec2& expectedPosition,
                          const float maxDistanceSquared, const billiard::search::State& state);
}

TEST(nextSearch, should_get_all_colors_if_previous_was_red_and_there_are_more_red_balls) {

    std::vector<billiard::search::Ball> balls {
        billiard::search::Ball{glm::vec2{0, 0}, "RED", "id-1"}
    };

    // Act
    auto result = billiard::snooker::nextSearch(billiard::search::State{balls}, std::vector<std::string>{"RED"});

    // Assert
    ASSERT_EQ(6, result._types.size());
    ASSERT_TRUE(test::util::contains<std::string>("BLACK", result._types));
    ASSERT_TRUE(test::util::contains<std::string>("PINK", result._types));
    ASSERT_TRUE(test::util::contains<std::string>("BLUE", result._types));
    ASSERT_TRUE(test::util::contains<std::string>("BROWN", result._types));
    ASSERT_TRUE(test::util::contains<std::string>("GREEN", result._types));
    ASSERT_TRUE(test::util::contains<std::string>("YELLOW", result._types));
}

TEST(nextSearch, should_get_yellow_if_previous_was_red_and_there_are_no_more_red_balls) {
    std::vector<billiard::search::Ball> balls {};

    // Act
    auto result = billiard::snooker::nextSearch(billiard::search::State{balls}, std::vector<std::string>{"RED"});

    // Assert
    ASSERT_EQ(1, result._types.size());
    ASSERT_TRUE(test::util::contains<std::string>("YELLOW", result._types));
}

TEST(nextSearch, should_get_red_if_previous_was_color_and_there_is_a_red_ball) {
    std::vector<billiard::search::Ball> balls {
            billiard::search::Ball{glm::vec2{0, 0}, "RED", "id-1"}
    };

    // Act
    auto result = billiard::snooker::nextSearch(billiard::search::State{balls},
                                                std::vector<std::string>{"BLACK", "PINK", "BROWN", "BLUE", "GREEN", "YELLOW"});

    // Assert
    ASSERT_EQ(1, result._types.size());
    ASSERT_TRUE(test::util::contains<std::string>("RED", result._types));
}


template <class Precondition, class Expected>
struct TestParam {
    Precondition _precondition;
    Expected _expected;
};

class NextSearchColorTest :public ::testing::TestWithParam<TestParam<std::string, std::string>> {
};

TEST_P(NextSearchColorTest, should_get_next_color_if_previous_was_color_and_there_is_no_red_ball) {
    const auto testParam = GetParam();

    std::vector<billiard::search::Ball> balls {};

    // Act
    auto result = billiard::snooker::nextSearch(billiard::search::State{balls},
                                                std::vector<std::string>{testParam._precondition});

    // Assert
    ASSERT_EQ(1, result._types.size());
    ASSERT_TRUE(test::util::contains<std::string>(testParam._expected, result._types));
}

INSTANTIATE_TEST_CASE_P(
        nextSearch,
        NextSearchColorTest,
        ::testing::Values(
                TestParam<std::string, std::string>{"YELLOW", "GREEN"},
                TestParam<std::string, std::string>{"GREEN", "BROWN"},
                TestParam<std::string, std::string>{"BROWN", "BLUE"},
                TestParam<std::string, std::string>{"BLUE", "PINK"},
                TestParam<std::string, std::string>{"PINK", "BLACK"}
        ));

const glm::vec2 yellowSpot {-600, 200};
const glm::vec2 greenSpot {-600, -200};
const glm::vec2 brownSpot {-600, 0};
const glm::vec2 blueSpot {0, 0};
const glm::vec2 pinkSpot {580, 0};
const glm::vec2 blackSpot {893.8, 0};

const std::unordered_map<std::string, std::string> coloredBallIds {
        {"YELLOW", "id-1"},
        {"GREEN", "id-2"},
        {"BROWN", "id-3"},
        {"BLUE", "id-4"},
        {"PINK", "id-5"},
        {"BLACK", "id-6"},
};

std::unordered_map<std::string, billiard::snooker::Spot> spots {
        {"YELLOW", billiard::snooker::Spot{yellowSpot}},
        {"GREEN", billiard::snooker::Spot{greenSpot}},
        {"BROWN", billiard::snooker::Spot{brownSpot}},
        {"BLUE", billiard::snooker::Spot{blueSpot}},
        {"PINK", billiard::snooker::Spot{pinkSpot}},
        {"BLACK", billiard::snooker::Spot{blackSpot}}
};

void initializeReplacement();

TEST(stateAfterBreak, should_do_nothing_if_all_colored_balls_are_set) {
    std::vector<billiard::search::Ball> balls {
            billiard::search::Ball{glm::vec2{0, 0}, "YELLOW", coloredBallIds.at("YELLOW")},
            billiard::search::Ball{glm::vec2{1, 1}, "GREEN", coloredBallIds.at("GREEN")},
            billiard::search::Ball{glm::vec2{2, 2}, "BROWN", coloredBallIds.at("BROWN")},
            billiard::search::Ball{glm::vec2{3, 3}, "BLUE", coloredBallIds.at("BLUE")},
            billiard::search::Ball{glm::vec2{4, 4}, "PINK", coloredBallIds.at("PINK")},
            billiard::search::Ball{glm::vec2{5, 5}, "BLACK", coloredBallIds.at("BLACK")}
    };

    // Act
    auto result = billiard::snooker::stateAfterBreak(billiard::search::State{balls}, coloredBallIds);

    // Assert
    ASSERT_EQ(balls, result._balls);
}

class StateAfterBreakReplaceBallSpotTest :public ::testing::TestWithParam<TestParam<std::string, glm::vec2>> {
};

TEST_P(StateAfterBreakReplaceBallSpotTest, should_replace_ball_at_expected_position_if_there_is_a_red_ball) {
    initializeReplacement();

    const auto testParam = GetParam();
    std::vector<billiard::search::Ball> balls;
    balls.emplace_back(billiard::search::Ball{glm::vec2{-1000, -1000}, "RED", "id-RED-1"});
    for(auto& coloredBall : coloredBallIds) {
        if (coloredBall.first == testParam._precondition) {
            continue;
        }
        balls.emplace_back(billiard::search::Ball{spots.at(coloredBall.first)._position,
                                               coloredBall.first,
                                               coloredBall.second});
    }


    // Act
    auto result = billiard::snooker::stateAfterBreak(billiard::search::State{balls}, coloredBallIds);

    // Assert
    ASSERT_EQ(7, result._balls.size());
    ASSERT_TRUE(test::util::ballIsAtPosition(testParam._precondition, testParam._expected, result));
}

INSTANTIATE_TEST_CASE_P(
        stateAfterBreak,
        StateAfterBreakReplaceBallSpotTest,
        ::testing::Values(
                TestParam<std::string, glm::vec2>{"YELLOW", yellowSpot},
                TestParam<std::string, glm::vec2>{"GREEN", greenSpot},
                TestParam<std::string, glm::vec2>{"BROWN", brownSpot},
                TestParam<std::string, glm::vec2>{"BLUE", blueSpot},
                TestParam<std::string, glm::vec2>{"PINK", pinkSpot},
                TestParam<std::string, glm::vec2>{"BLACK", blackSpot}
        ));

class StateAfterBreakReplaceBallSpotOccupiedTest :public ::testing::TestWithParam<TestParam<std::string, glm::vec2>> {
};

TEST_P(StateAfterBreakReplaceBallSpotOccupiedTest, should_replace_ball_at_expected_position_if_there_is_a_red_ball_and_the_ball_spot_is_occupied) {
    initializeReplacement();

    const auto testParam = GetParam();
    std::vector<billiard::search::Ball> balls;
    for(auto& coloredBall : coloredBallIds) {
        auto position = spots.at(coloredBall.first)._position;

        if (coloredBall.first == testParam._precondition) {
            balls.emplace_back(billiard::search::Ball{position, "RED", "id-RED-1"});
            continue;
        }

        if (position == testParam._expected) {
            position = glm::vec2{-1000, -1000};
        }

        balls.emplace_back(billiard::search::Ball{position,
                                                  coloredBall.first,
                                                  coloredBall.second});
    }


    // Act
    auto result = billiard::snooker::stateAfterBreak(billiard::search::State{balls}, coloredBallIds);

    // Assert
    ASSERT_EQ(7, result._balls.size());
    ASSERT_TRUE(test::util::ballIsAtPosition(testParam._precondition, testParam._expected, result));
}

INSTANTIATE_TEST_CASE_P(
        stateAfterBreak,
        StateAfterBreakReplaceBallSpotOccupiedTest,
        ::testing::Values(
                TestParam<std::string, glm::vec2>{"YELLOW", greenSpot},
                TestParam<std::string, glm::vec2>{"YELLOW", brownSpot},
                TestParam<std::string, glm::vec2>{"YELLOW", blueSpot},
                TestParam<std::string, glm::vec2>{"YELLOW", pinkSpot},
                TestParam<std::string, glm::vec2>{"YELLOW", blackSpot},
                TestParam<std::string, glm::vec2>{"GREEN", brownSpot},
                TestParam<std::string, glm::vec2>{"GREEN", blueSpot},
                TestParam<std::string, glm::vec2>{"GREEN", pinkSpot},
                TestParam<std::string, glm::vec2>{"GREEN", blackSpot},
                TestParam<std::string, glm::vec2>{"BROWN", blueSpot},
                TestParam<std::string, glm::vec2>{"BROWN", pinkSpot},
                TestParam<std::string, glm::vec2>{"BROWN", blackSpot},
                TestParam<std::string, glm::vec2>{"BLUE", pinkSpot},
                TestParam<std::string, glm::vec2>{"BLUE", blackSpot},
                TestParam<std::string, glm::vec2>{"PINK", blackSpot}
        ));

TEST(stateAfterBreak, should_replace_ball_next_to_spot_if_all_spots_are_occupied) {
    initializeReplacement();

    std::vector<billiard::search::Ball> balls {
            billiard::search::Ball{yellowSpot, "RED", "id-RED-1"},
            billiard::search::Ball{greenSpot, "GREEN", coloredBallIds.at("GREEN")},
            billiard::search::Ball{brownSpot, "BROWN", coloredBallIds.at("BROWN")},
            billiard::search::Ball{blueSpot, "BLUE", coloredBallIds.at("BLUE")},
            billiard::search::Ball{pinkSpot, "PINK", coloredBallIds.at("PINK")},
            billiard::search::Ball{blackSpot, "BLACK", coloredBallIds.at("BLACK")}
    };

    static float maxDistance = 26.15f * SPOT_REPLACE_RADIUS_MULTIPLIER;
    static float maxDistanceSquared = maxDistance * maxDistance;

    // Act
    auto result = billiard::snooker::stateAfterBreak(billiard::search::State{balls}, coloredBallIds);

    // Assert
    ASSERT_EQ(7, result._balls.size());
    ASSERT_TRUE(test::util::ballIsNextToPosition("YELLOW", yellowSpot, maxDistanceSquared, result));
}

TEST(stateAfterBreak, should_replace_black_next_to_spot_on_table_if_all_spots_are_occupied) {
    initializeReplacement();

    std::vector<billiard::search::Ball> balls {
            billiard::search::Ball{yellowSpot, "YELLOW", coloredBallIds.at("BLACK")},
            billiard::search::Ball{greenSpot, "GREEN", coloredBallIds.at("GREEN")},
            billiard::search::Ball{brownSpot, "BROWN", coloredBallIds.at("BROWN")},
            billiard::search::Ball{blueSpot, "BLUE", coloredBallIds.at("BLUE")},
            billiard::search::Ball{pinkSpot, "PINK", coloredBallIds.at("PINK")},
            billiard::search::Ball{blackSpot, "RED", "id-RED-1"}
    };

    static float maxDistance = 26.15f * SPOT_REPLACE_RADIUS_MULTIPLIER;
    static float maxDistanceSquared = maxDistance * maxDistance;

    // Act
    auto result = billiard::snooker::stateAfterBreak(billiard::search::State{balls}, coloredBallIds);

    // Assert
    ASSERT_EQ(7, result._balls.size());
    ASSERT_TRUE(test::util::ballIsNextToPosition("BLACK", blackSpot, maxDistanceSquared, result));
}

void initializeReplacement() {
    glm::vec2 headRailDirection{1, 0};
    float radius = 26.15f;
    float diameterSquared = (radius * 2.0f) * (radius * 2.0f);

    billiard::snooker::SnookerSearchConfig config {
        spots, headRailDirection, diameterSquared, radius,
        glm::vec2(-1881.0f/2.0f, -943.0f/2.0f),
        glm::vec2(1881.0f/2.0f, 943.0f/2.0f)
    };
    billiard::snooker::searchConfig(config);
}

template<class T>
bool test::util::contains(const T& expected, const std::vector<T>& check) {
    return std::find(check.begin(), check.end(), expected) != check.end();
}

bool test::util::ballIsAtPosition(const std::string& type, const glm::vec2& expectedPosition,
                          const billiard::search::State& state) {
    for (auto& ball : state._balls) {
        if (ball._type == type) {
            return ball._position == expectedPosition;
        }
    }
    return false;
}

bool test::util::ballIsNextToPosition(const std::string& type, const glm::vec2& expectedPosition,
                                  const float maxDistanceSquared, const billiard::search::State& state) {
    for (auto& ball : state._balls) {
        if (ball._type == type) {
            auto vec = expectedPosition - ball._position;
            return glm::dot(vec, vec) <= maxDistanceSquared;
        }
    }
    return false;
}