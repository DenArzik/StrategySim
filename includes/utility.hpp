#pragma once

#include <array>
#include <chrono>
#include <random>
#include <concepts>

#include <iostream>

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

namespace RNG
{
    // https://stackoverflow.com/questions/1046714/what-is-a-good-random-number-generator-for-a-game

    using rng_type = unsigned long;
    static rng_type x=123456789, y=362436069, z=521288629;

    static rng_type xorshf96()
    {          
        rng_type t;
        x ^= x << 16;
        x ^= x >> 5;
        x ^= x << 1;

        t = x;
        x = y;
        y = z;
        z = t ^ x ^ y;

        return z;
    }

    rng_type random_uint()
    {
        return xorshf96();
    }


    template<typename T>
    concept UnsignedIntegral = std::is_integral<T>::value && std::is_unsigned<T>::value;

    template<UnsignedIntegral T>
    rng_type random_uniform_uint(T min, T max)
    {
        const auto range {max - min + 1};
        const auto normalized_value{random_uint() % range};        
        return normalized_value + min;
    }
}

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
