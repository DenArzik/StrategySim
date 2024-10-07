#pragma once

#include <array>
#include <chrono>

#include <iostream>

#include "rng.hpp"

template<typename T, std::size_t N>
class FixedSizeVector
{
public:
    using Underlying = std::array<T, N>;
    using CUnderlying = std::array<const T, N>;

    void push_back(const T &obj)
    {
        m_arr[m_last_elem_idx++] = obj;
    }

    Underlying::iterator begin()
    {
        return m_arr.begin();
    }

    const CUnderlying::iterator begin() const
    {
        return m_arr.cbegin();
    }

    Underlying::iterator end()
    {
        return begin() + m_last_elem_idx;
    }

    const CUnderlying::iterator end() const
    {
        return begin() + m_last_elem_idx;
    }

    const T &operator[](std::size_t idx) const
    {
        return m_arr[idx];
    }

    std::size_t size() const 
    {
        return m_last_elem_idx;
    }

    bool empty() const
    {
        return m_last_elem_idx == 0;
    }

private:
    Underlying m_arr;
    std::size_t m_last_elem_idx{0};
};


namespace Timing
{

template<typename Precision>
class ScopedTimeMeasurer
{
public:
    ScopedTimeMeasurer(const char *tag = "Undefined") 
        : m_begin{std::chrono::high_resolution_clock::now()} 
        , m_tag{tag} 
    {}

    ~ScopedTimeMeasurer()
    {
        const auto end{get_time()};
        const auto duration {std::chrono::duration_cast<Precision>(end-m_begin)};

        std::cout << "[" << m_tag << "] took " << duration << std::endl;
    }

    using time_point = decltype(std::chrono::high_resolution_clock::now());

    static time_point get_time()
    {
        return std::chrono::high_resolution_clock::now();
    }

private:
    const std::chrono::high_resolution_clock::time_point m_begin;
    const char *m_tag;

};

}
