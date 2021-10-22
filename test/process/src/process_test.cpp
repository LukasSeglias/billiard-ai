#include <gtest/gtest.h>
#include <process/process.hpp>
#include <memory>
#include <iostream>
#include <random>

#define EXPANDS 10

struct Data {
    uint8_t _level;
    std::shared_ptr<Data> _parent;
    uint64_t _result;

    // Interface
    uint64_t _cost;
    bool _isSolution;
};

struct Config {
    uint8_t _testValue;
};

void runTest(process::ProcessManager<std::shared_ptr<Data>, Config, uint64_t>& processManager, uint64_t& totalTime,
             uint64_t& totalSolutions);
std::vector<std::shared_ptr<Data>> expand(const std::shared_ptr<Data>& input, const std::shared_ptr<Config>& config);
uint64_t mapToSolution(const std::shared_ptr<Data>& data);
std::vector<uint64_t> sortAscending(const std::vector<uint64_t>& data);

TEST(ProcessManager, SHOULD_terminate_IF_manager_deleted) {
    process::ProcessManager<std::shared_ptr<Data>, Config, uint64_t> processManager{2, 5, expand, mapToSolution};

    Data data{0, nullptr, 0, 0,false};

    auto result = processManager.process(std::vector<std::shared_ptr<Data>>{
            std::make_shared<Data>(data),
            std::make_shared<Data>(data),
            std::make_shared<Data>(data)
    });
}

TEST(ProcessManager, SHOULD_ignore_previous_tasks_IF_new_task_started) {
    std::this_thread::sleep_for(std::chrono::seconds(5));
    process::ProcessManager<std::shared_ptr<Data>, Config, uint64_t> processManager{6, 1, expand, mapToSolution};

    Data data{0, nullptr, 0, 0,false};
    const uint16_t tests = 10000;

    for(int i = 0; i < tests; i++) {
        std::cout << "-------- Run test (" << i + 1 << ")--------" << std::endl;
        auto result = processManager.process(std::vector<std::shared_ptr<Data>>{
                std::make_shared<Data>(data),
                std::make_shared<Data>(data),
                std::make_shared<Data>(data)
        });
        if (i == tests - 1) {
            result.wait();
            auto solutions = result.get();
            auto sortedAscending = sortAscending(solutions);

            for (int j = 0; j < solutions.size(); j++) {
                auto solution = solutions.at(j);
                auto expected = sortedAscending.at(j);
                ASSERT_EQ(expected, solution);
            }
        }
        std::cout << "---------------------------------------\n" << std::endl << std::flush;
    }
}

TEST(ProcessManager, SHOULD_terminate_earlier_IF_it_has_nothing_to_work) {
    process::ProcessManager<std::shared_ptr<Data>, Config, uint64_t> processManager{6, 1, expand, mapToSolution};

    Data data{4, nullptr, 0, 0,false};

    uint64_t tick = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    auto result = processManager.process(std::vector<std::shared_ptr<Data>>{
            std::make_shared<Data>(data),
            std::make_shared<Data>(data)
    });

    result.wait();
    uint64_t tack = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    uint64_t time = tack - tick;
    std::cout << "Time: " << time << std::endl;
    auto solutions = result.get();
    auto sortedAscending = sortAscending(solutions);

    ASSERT_LT(time, 10000);
    ASSERT_EQ(2 * EXPANDS, solutions.size());
    for (int j = 0; j < solutions.size(); j++) {
        auto solution = solutions.at(j);
        auto expected = sortedAscending.at(j);
        ASSERT_EQ(expected, solution);
    }
}

TEST(ProcessManager, SHOULD_fulfill_all_tasks) {
    process::ProcessManager<std::shared_ptr<Data>, Config, uint64_t> processManager{6, 40, expand, mapToSolution};
    std::cout << "Wait a few seconds before start\n" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));

    uint64_t totalTime = 0;
    uint64_t totalSolutions = 0;
    bool wait = false;

    const uint16_t tests = 1000;

    for(int i = 0; i < tests; i++) {
        std::cout << "-------- Run test (" << i + 1 << ")--------" << std::endl;
        runTest(processManager, totalTime, totalSolutions);

        if (wait) {
            std::cout << "Let the manager run with no work to do..." << std::endl;
            for (int j = 1 + (i % 10); j > 0; j--) {
                std::cout << "Go on in " << j << " seconds" << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }

        if ((i + 1) % 50 == 0) {
            wait = !wait;
        }
        std::cout << "---------------------------------------\n" << std::endl << std::flush;
    }

    std::cout << "-------------Results--------------" << std::endl;
    std::cout << "Total Solutions: " << totalSolutions << "    Total Time [ms]: " << totalTime << std::endl;
    std::cout << "Solutions per test: " << (totalSolutions / tests) << "     Time per test [ms]: " << (totalTime / tests) << std::endl;
    std::cout << "-------------End Results--------------\n" << std::endl;
}

void runTest(process::ProcessManager<std::shared_ptr<Data>, Config, uint64_t>& processManager, uint64_t& totalTime,
             uint64_t& totalSolutions) {
    Data data{0, nullptr, 0, 0,false};

    // Act
    uint64_t tick = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    auto result = processManager.process(std::vector<std::shared_ptr<Data>>{
            std::make_shared<Data>(data),
            std::make_shared<Data>(data),
            std::make_shared<Data>(data)
    }, 10, std::make_shared<Config>(Config{10}), 40000);

    // Assert
    result.wait();
    uint64_t tack = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    uint64_t time = tack - tick;
    std::cout << "Computation time [ms]: " << time << std::endl;
    totalTime += time;

    auto solutions = result.get();
    totalSolutions += solutions.size();
    std::cout << "Solutions: " << solutions.size() << std::endl;
    auto sortedAscending = sortAscending(solutions);

    for (int i = 0; i < solutions.size(); i++) {
        auto solution = solutions.at(i);
        auto expected = sortedAscending.at(i);
        ASSERT_EQ(expected, solution);
    }
}

// Seed with a real random value, if available
std::random_device r;
// Choose a random mean between 1 and 6
std::default_random_engine e1(r());
std::uniform_int_distribution<int> uniform_dist(1, 10);
std::uniform_int_distribution<int> calculation_time(10, 15);
std::uniform_int_distribution<int> boolean(0, 1);

std::shared_ptr<Data> expandOne(const std::shared_ptr<Data>& input) {
    uint64_t randomVal = uniform_dist(e1);

    bool add = boolean(e1);
    uint64_t lowerBound = 0;
    uint64_t result = std::max(add ? input->_result + randomVal : input->_result - randomVal, lowerBound);
    uint8_t level = input->_level + 1;

    uint64_t wait = calculation_time(e1);
    std::this_thread::sleep_for(std::chrono::milliseconds(wait));

    return std::make_shared<Data>(Data{level, input, result, result, level == 5});
}

std::vector<std::shared_ptr<Data>> expand(const std::shared_ptr<Data>& input, const std::shared_ptr<Config>& config) {
    std::vector<std::shared_ptr<Data>> expanded;

    for(int i = 0; i < EXPANDS; i++) {
        expanded.push_back(expandOne(input));
    }

    return expanded;
}

uint64_t mapToSolution(const std::shared_ptr<Data>& data) {
    return data->_cost;
}

std::vector<uint64_t> sortAscending(const std::vector<uint64_t>& data) {
    std::vector<uint64_t> sorted{data};
    std::sort(sorted.begin(), sorted.end());
    return sorted;
}