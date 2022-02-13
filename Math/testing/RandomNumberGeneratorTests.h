#pragma once

#include "Math/RandomNumberGenerator.h"

/// A namespace for testing the code in the corresponding class.
namespace RANDOM_NUMBER_GENERATOR_TESTS
{
    TEST_CASE("A random number can be generated less than a particular value", "[RandomNumberGenerator]")
    {
        MATH::RandomNumberGenerator random_number_generator;

        constexpr int EXCLUDED_MAX = 100;
        int random_number = random_number_generator.RandomNumberLessThan<int>(EXCLUDED_MAX);
        REQUIRE(random_number < EXCLUDED_MAX);
    }

    TEST_CASE("A random number can be generated within a range", "[RandomNumberGenerator]")
    {
        MATH::RandomNumberGenerator random_number_generator;

        constexpr int MIN = 12;
        constexpr int MAX = 78;
        int random_number = random_number_generator.RandomInRange<int>(MIN, MAX);
        REQUIRE(random_number >= MIN);
        REQUIRE(random_number <= MAX);
    }
}
