#pragma once

#include <chrono>
#include <iostream>
#include <functional>
#include <ranges>

template <size_t Size>
class timer {
    std::array<std::chrono::steady_clock::time_point, Size> m_start;
    std::array<std::chrono::duration<double>, Size> m_duration{};
    std::array<bool, Size> is_paused{}; // default false
public:

    timer() {
        for (auto& start : m_start) {
            start = std::chrono::high_resolution_clock::now();
        }
    }

    void pause(size_t idx) {
        if (is_paused) { throw std::runtime_error("Timer is paused when *.pause() was called"); } 
        m_duration[idx] += std::chrono::high_resolution_clock::now() - m_start;
    }

    void unpause(size_t idx) {
        if (!is_paused) { throw std::runtime_error("Timer is unpaused when *.unpause() was called"); } 
        m_start[idx] = std::chrono::high_resolution_clock::now();
    }

    ~timer() {
        for (auto i : std::views::iota(0uz, Size)) {
            if (!is_paused) { (*this).pause(); }
            std::cout << std::format("Clock #{}; Execution time: {} seconds\n", i, m_duration.count());
        }
    }
};