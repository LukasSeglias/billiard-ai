#pragma once

#include <thread>
#include <future>
#include <queue>
#include <vector>
#include <chrono>

namespace process {

    #define SYNC_STEP 2

    template <class Data, class Parameter, class Solution>
    class Process;

    template <class Data>
    class Comparator {
    public:
        bool operator() (const Data& data1, const Data& data2) {
            return data1->_cost > data2->_cost;
        }
    };

    template<class Data, class Parameter>
    struct RequestData {
        std::vector<Data> _data;
        std::shared_ptr<Parameter> _parameter;
    };

    template <class Data>
    using priority_queue = std::priority_queue<Data, std::vector<Data>, Comparator<Data>>;

    template <class Data, class Parameter>
    using expand_function = std::function<std::vector<Data>(const Data&, const std::shared_ptr<Parameter>&)>;
    template <class Data, class Solution>
    using map_solution_function = std::function<Solution (const Data&)>;

    template <class Data, class Parameter, class Solution>
    class ProcessManager {
    public:
        ProcessManager(uint8_t processes, uint64_t syncPeriod, const expand_function<Data, Parameter>& expand,
                       const map_solution_function<Data, Solution>& mapSolution) :
            _exitSignal(),
            _inProgress(),
            _solutions(),
            _lock(),
            _syncPeriod(std::max(syncPeriod, static_cast<uint64_t>(SYNC_STEP * processes))),
            _mapSolution(mapSolution),
            _result(),
            _dataRequests(),
            _startTime(0),
            _minimalSolutions(100),
            _startedNew(false),
            _isRunning(false),
            _maxDuration(10000),
            _workers(buildProcesses(processes, _syncPeriod, expand, this)),
            _worker(std::thread(ProcessManager::work, this)) {

        }

        ProcessManager(const ProcessManager<Data, Parameter, Solution>& other) = delete;
        ProcessManager(ProcessManager<Data, Parameter, Solution>&& other) = delete;
        ProcessManager<Data, Parameter, Solution>& operator=(const ProcessManager<Data, Parameter, Solution>& other) = delete;
        ProcessManager<Data, Parameter, Solution>& operator=(ProcessManager<Data, Parameter, Solution>&& other) = delete;
        ~ProcessManager() {
            _exitSignal.set_value();
            _worker.join();

            _workers.clear();
        }

        friend class Process<Data, Parameter, Solution>;

        std::future<std::vector<Solution>>
        process(const std::vector<Data>& input, uint16_t minimalSolutions = 100,
                const std::shared_ptr<Parameter> expandParameter = nullptr, uint64_t maxDuration = 10000) {
            _lock.lock();
            clear();

            _minimalSolutions = minimalSolutions;
            _result = std::promise<std::vector<Solution>>{};
            _expandParameter = expandParameter;
            _inProgress = priority_queue<Data>{input.begin(), input.end()};

            _startTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count();
            _maxDuration = maxDuration;
            _startedNew = true;
            _isRunning = true;

            _lock.unlock();
            return _result.get_future();
        }

    private:
        std::promise<void> _exitSignal;
        priority_queue<Data> _inProgress;
        priority_queue<Data> _solutions;
        std::mutex _lock;
        uint64_t _syncPeriod;
        map_solution_function<Data, Solution> _mapSolution;
        std::promise<std::vector<Solution>> _result;
        std::vector<std::promise<RequestData<Data, Parameter>>> _dataRequests;
        uint64_t _startTime;
        uint16_t _minimalSolutions;
        std::atomic_bool _startedNew;
        std::atomic_bool _isRunning;
        uint64_t _maxDuration;
        std::shared_ptr<Parameter> _expandParameter;
        std::vector<std::shared_ptr<Process<Data, Parameter, Solution>>> _workers;
        std::thread _worker;

        static std::vector<std::shared_ptr<Process<Data, Parameter, Solution>>>
        buildProcesses(uint8_t processes, uint16_t syncPeriod, const expand_function<Data, Parameter>& expand,
                       ProcessManager<Data, Parameter, Solution>* const processManager) {
            std::vector<std::shared_ptr<Process<Data, Parameter, Solution>>> processVector;

            for (int i = 0; i < processes; i++) {
                processVector.push_back(
                        std::make_shared<Process<Data, Parameter, Solution>>(syncPeriod, expand, i, processManager));
            }

            return processVector;
        }

        static void work(ProcessManager<Data, Parameter, Solution>* manager) {
            auto exitListener = manager->_exitSignal.get_future();
            auto lastRun = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count() + manager->_workers.size() * SYNC_STEP;

            while (exitListener.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout) {

                auto currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now().time_since_epoch()).count();
                if (manager->_isRunning && (currentTime - lastRun >= manager->_syncPeriod || manager->_startedNew)) {
                    manager->_startedNew = false;
                    lastRun = currentTime;

                    manager->_lock.lock();

                    mayCompleteResult(manager);
                    mayCompleteRequests(manager);

                    manager->_lock.unlock();
                }
            }
        }

        static void mayCompleteRequests(ProcessManager<Data, Parameter, Solution>* const manager) {
            for (auto& process : manager->_workers) {
                if (process->_clearFlag) {
                    return;
                }
            }

            if (!manager->_dataRequests.empty()) {
                std::vector<std::vector<Data>> forRequests{manager->_dataRequests.size()};

                uint16_t currentRequest = 0;
                while (!manager->_inProgress.empty()) {
                    forRequests.at(currentRequest).push_back(manager->_inProgress.top());
                    manager->_inProgress.pop();
                    currentRequest = (currentRequest + 1) % manager->_dataRequests.size();
                }

                auto iterator = manager->_dataRequests.begin();
                currentRequest = 0;
                while(iterator != manager->_dataRequests.end()) {
                    if (!forRequests.at(currentRequest).empty()) {
                        iterator->set_value(RequestData<Data, Parameter>{forRequests.at(currentRequest),
                                                                         manager->_expandParameter});
                        iterator = manager->_dataRequests.erase(iterator);
                    } else {
                        iterator++;
                    }
                    currentRequest++;
                }
            }
        }

        static void mayCompleteResult(ProcessManager<Data, Parameter, Solution>* const manager) {
            uint64_t currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count();
            bool finished = manager->_isRunning && (manager->_solutions.size() >= manager->_minimalSolutions ||
                            (currentTime - manager->_startTime >= manager->_maxDuration));
            if (finished) {
                std::vector<Solution> result;
                while(!manager->_solutions.empty()) {
                    result.push_back(manager->_mapSolution(manager->_solutions.top()));
                    manager->_solutions.pop();
                }
                manager->clear();
                manager->_result.set_value(result);
            }
        }

        void pushData(const Process<Data, Parameter, Solution>* const process, const std::vector<Data>& inProgress,
                      const std::vector<Data>& solutions) {
            _lock.lock();
            if (!process->_clearFlag) {
                for(auto& data : inProgress) {
                    _inProgress.push(data);
                }
                for(auto& data : solutions) {
                    _solutions.push(data);
                }
            }
            _lock.unlock();
        }

        std::future<RequestData<Data, Parameter>>
        requestData() {
            std::promise<RequestData<Data, Parameter>> promise;
            auto future = promise.get_future();

            _lock.lock();
            _dataRequests.emplace_back(std::move(promise));
            _lock.unlock();

            return future;
        }

        void clear() {
            for (auto worker : _workers) {
                worker->clear();
            }
            _inProgress = priority_queue<Data>{};
            _solutions = priority_queue<Data>{};
            _isRunning = false;
        }
    };

    template <class Data, class Parameter, class Solution>
    class Process {
    public:

        Process(uint64_t syncPeriod, const expand_function<Data, Parameter>& expand,
                uint8_t id, ProcessManager<Data, Parameter, Solution>* const processManager) :
                _clearFlag(false),
                _exitSignal(),
                _inProgress(),
                _solutions(),
                _syncPeriod(syncPeriod),
                _expand(expand),
                _processManager(processManager),
                _dataRequest(),
                _lastSynced(std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now().time_since_epoch()).count() + (id * SYNC_STEP)),
                _expandParameter(nullptr),
                _worker(std::thread(Process::work, this)) {
        }

        ~Process() {
            _exitSignal.set_value();
            _worker.join();
        }

        void clear() {
            _clearFlag = true;
        }

        friend class ProcessManager<Data, Parameter, Solution>;

    private:
        std::atomic_bool _clearFlag;
        std::promise<void> _exitSignal;
        priority_queue<Data> _inProgress;
        priority_queue<Data> _solutions;
        uint64_t _syncPeriod;
        expand_function<Data, Parameter> _expand;
        ProcessManager<Data, Parameter, Solution>* const _processManager;
        std::future<RequestData<Data, Parameter>> _dataRequest;
        uint64_t _lastSynced;
        std::shared_ptr<Parameter> _expandParameter;
        std::thread _worker;

        static void work(Process<Data, Parameter, Solution>* const process) {
            auto exitListener = process->_exitSignal.get_future();

            uint8_t cyclesSinceLastDataCheck = 0;
            while (exitListener.wait_for(std::chrono::nanoseconds (0)) == std::future_status::timeout) {
                mayClear(process);
                mayAskForData(process);
                cyclesSinceLastDataCheck = mayPushData(process, cyclesSinceLastDataCheck);
                mayAppendDataFromRequest(process);
                expand(process);
                cyclesSinceLastDataCheck++;
            }
        }

        static void mayClear(Process<Data, Parameter, Solution>* const process) {
            if (process->_clearFlag) {
                mayAppendDataFromRequest(process);

                process->_inProgress = priority_queue<Data>{};
                process->_solutions = priority_queue<Data>{};

                process->_clearFlag = false;
            }
        }

        static void mayAskForData(Process<Data, Parameter, Solution>* const process) {
            if (!process->_dataRequest.valid()) {
                process->_dataRequest = process->_processManager->requestData();
            }
        }

        static uint8_t
        mayPushData(Process<Data, Parameter, Solution>* const process, uint8_t cyclesSinceLastDataCheck) {
            if (cyclesSinceLastDataCheck < 100) {
                return cyclesSinceLastDataCheck;
            }

            uint64_t currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count();
            if (currentTime - process->_lastSynced > process->_syncPeriod) {
                process->_lastSynced = currentTime;

                auto synchronizeInProgress = getForSynchronization(process->_inProgress, 0.6);
                auto synchronizeSolutions = getForSynchronization(process->_solutions, 1);

                process->_processManager->pushData(process, synchronizeInProgress, synchronizeSolutions);
                return 0;
            }
            return cyclesSinceLastDataCheck;
        }

        static void mayAppendDataFromRequest(Process<Data, Parameter, Solution>* const process) {
            if (process->_dataRequest.valid() &&
                process->_dataRequest.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                auto result = process->_dataRequest.get();
                for (auto& data : result._data) {
                    process->_inProgress.push(data);
                }
                process->_expandParameter = result._parameter;
            }
        }

        static void expand(Process<Data, Parameter, Solution>* const process) {
            if (process->_inProgress.empty()) {
                return;
            }

            auto data = process->_inProgress.top();
            process->_inProgress.pop();
            for(auto& expanded : process->_expand(data, process->_expandParameter)) {
                if (expanded->_isSolution) {
                    process->_solutions.push(expanded);
                } else {
                    process->_inProgress.push(expanded);
                }
            }
        }

        static std::vector<Data> getForSynchronization(priority_queue<Data>& queue, double percentage) {
            std::vector<Data> dataForSynchronization;

            uint32_t amountOfDataForSynchronization = std::ceil(queue.size() * percentage);
            for(uint32_t i = 0; i < amountOfDataForSynchronization; i++) {
                if (queue.empty()) {
                    break;
                }
                dataForSynchronization.push_back(queue.top());
                queue.pop();
            }
            return dataForSynchronization;
        }
    };
}