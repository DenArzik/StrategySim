#pragma once

#include <random>
#include <concepts>
#include <type_traits>

namespace RNG
{

using rng_uint = unsigned long int;

template<typename T>
concept UnsignedIntegral = std::is_integral<T>::value && std::is_unsigned<T>::value;

class RandomDeviceSeedGenerator
{
public:
    static rng_uint generate()
    {
        std::random_device dev;
        return dev();
    }
};


class MtEngine
{
public:
    MtEngine(rng_uint seed)
    : m_rng{seed}
    {
    }

    rng_uint generate_number()
    {
        return m_rng();
    }

    using EngineType = std::conditional<
        sizeof(rng_uint) == sizeof(uint64_t)
        , std::mt19937_64
        , std::mt19937
        >::type;

    EngineType &get_engine()
    {
        return m_rng;
    }


private:
    EngineType m_rng;
};


class DivDistributor
{
public:
    template<typename Generator, UnsignedIntegral T>
    static rng_uint distribute(Generator &g, T min, T max)
    {
        const auto range {max - min + 1};
        const auto normalized_value{g.generate_number() % range};
        return normalized_value + min;
    }
};

class StdUniformDistributor
{
public:
    template<typename Generator, UnsignedIntegral T>
    static rng_uint distribute(Generator &g, T min, T max)
    {
        std::uniform_int_distribution<> dist(min, max);
        return dist(g.get_engine());
    }
};

template<typename T>
concept SimpleEngine = requires(T t){
    t.generate_number();
};

template<typename SeedGenerator = RandomDeviceSeedGenerator
    , typename EngineGenerator = MtEngine
    , typename Distrubutor = DivDistributor>
class Generator
{
public:
    static Generator &get()
    {
        static Generator instance;
        return instance;
    }

    template<UnsignedIntegral T>
    rng_uint random_uniform_uint(T min, T max)
    {
        return Distrubutor::distribute(m_engine, min, max);
    }

    rng_uint random_uint()
    {
        return m_engine.generate_number();
    }


private:
    Generator()
    : m_engine{SeedGenerator::generate()}
    {
    }

    EngineGenerator m_engine;

};

}