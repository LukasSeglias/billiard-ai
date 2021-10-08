#include <billiard_debug/billiard_debug.hpp>
#include <iostream>

std::mutex billiard::debug::Debug::_mutexPrint{};

billiard::debug::Debug::~Debug() {
    std::lock_guard<std::mutex> guard(_mutexPrint);
    std::cout << this->str();
}

bool billiard::debug::Debug::lock() {
    _mutexPrint.lock();
    return true;
}

void billiard::debug::Debug::unlock() {
    _mutexPrint.unlock();
}