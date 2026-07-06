#pragma once

#include <chrono>
#include <iostream>
#include <functional>


class timer {
    std::chrono::steady_clock::time_point m_start;
    std::chrono::duration<double> m_duration{};
    bool is_paused { false };
public:

    timer() {
        m_start = std::chrono::high_resolution_clock::now();
    }

    void pause() {
        if (is_paused) { throw std::runtime_error("Timer is paused when *.pause() was called"); } 
        m_duration += std::chrono::high_resolution_clock::now() - m_start;
    }

    void unpause() {
        if (!is_paused) { throw std::runtime_error("Timer is unpaused when *.unpause() was called"); } 
        m_start = std::chrono::high_resolution_clock::now();
    }

    ~timer() {
        if (!is_paused) { (*this).pause(); }
        std::cout << std::format("Execution time: {} seconds\n", m_duration.count());
    }
};