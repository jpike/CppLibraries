#pragma once

#include "Math/Number.h"

/// A namespace for testing the code in the corresponding class.
namespace NUMBER_TESTS
{
    TEST_CASE("Even and odd numbers can be properly detected.", "[Number]")
    {
        bool one_is_even = MATH::Number::IsEven(1);
        REQUIRE(!one_is_even);

        bool two_is_even = MATH::Number::IsEven(2);
        REQUIRE(two_is_even);
    }

    TEST_CASE("Decrementing a number and wrapping within a range for a number greater than the range goes to the max.", "[Number]")
    {
        constexpr int NUMBER_TO_DECREMENT = 10;
        constexpr int MIN_VALUE = 5;
        constexpr int MAX_VALUE = 8;
        int result = MATH::Number::DecrementAndWrapWithinRange(
            NUMBER_TO_DECREMENT,
            MIN_VALUE,
            MAX_VALUE);
        REQUIRE(MAX_VALUE == result);
    }

    TEST_CASE("Decrementing a number and wrapping within a range for a number less than the range goes to the max.", "[Number]")
    {
        constexpr int NUMBER_TO_DECREMENT = 4;
        constexpr int MIN_VALUE = 5;
        constexpr int MAX_VALUE = 8;
        int result = MATH::Number::DecrementAndWrapWithinRange(
            NUMBER_TO_DECREMENT,
            MIN_VALUE,
            MAX_VALUE);
        REQUIRE(MAX_VALUE == result);
    }

    TEST_CASE("A value can be decremented within a range.", "[Number]")
    {
        constexpr int NUMBER_TO_DECREMENT = 7;
        constexpr int MIN_VALUE = 5;
        constexpr int MAX_VALUE = 8;
        int result = MATH::Number::DecrementAndWrapWithinRange(
            NUMBER_TO_DECREMENT,
            MIN_VALUE,
            MAX_VALUE);
        REQUIRE(6 == result);
    }

    TEST_CASE("Incrementing a number and wrapping within a range for a number less than the range goes to the min.", "[Number]")
    {
        constexpr int NUMBER_TO_INCREMENT = 3;
        constexpr int MIN_VALUE = 5;
        constexpr int MAX_VALUE = 8;
        int result = MATH::Number::IncrementAndWrapWithinRange(
            NUMBER_TO_INCREMENT,
            MIN_VALUE,
            MAX_VALUE);
        REQUIRE(MIN_VALUE == result);
    }

    TEST_CASE("Incrementing a number and wrapping within a range for a number greater than the range goes to the min.", "[Number]")
    {
        constexpr int NUMBER_TO_INCREMENT = 9;
        constexpr int MIN_VALUE = 5;
        constexpr int MAX_VALUE = 8;
        int result = MATH::Number::IncrementAndWrapWithinRange(
            NUMBER_TO_INCREMENT,
            MIN_VALUE,
            MAX_VALUE);
        REQUIRE(MIN_VALUE == result);
    }

    TEST_CASE("A value can be incremented within a range.", "[Number]")
    {
        constexpr int NUMBER_TO_INCREMENT = 6;
        constexpr int MIN_VALUE = 5;
        constexpr int MAX_VALUE = 8;
        int result = MATH::Number::IncrementAndWrapWithinRange(
            NUMBER_TO_INCREMENT,
            MIN_VALUE,
            MAX_VALUE);
        REQUIRE(7 == result);
    }

    TEST_CASE("A number smaller than a range can be clamped to the min.", "[Number]")
    {
        constexpr int NUMBER_TO_CLAMP = 4;
        constexpr int MIN_VALUE = 5;
        constexpr int MAX_VALUE = 8;
        int result = MATH::Number::Clamp(NUMBER_TO_CLAMP, MIN_VALUE, MAX_VALUE);
        REQUIRE(MIN_VALUE == result);
    }

    TEST_CASE("A number greater than a range can be clamped to the min.", "[Number]")
    {
        constexpr int NUMBER_TO_CLAMP = 9;
        constexpr int MIN_VALUE = 5;
        constexpr int MAX_VALUE = 8;
        int result = MATH::Number::Clamp(NUMBER_TO_CLAMP, MIN_VALUE, MAX_VALUE);
        REQUIRE(MAX_VALUE == result);
    }

    TEST_CASE("A number within a range is clamped to itself.", "[Number]")
    {
        constexpr int NUMBER_TO_CLAMP = 6;
        constexpr int MIN_VALUE = 5;
        constexpr int MAX_VALUE = 8;
        int result = MATH::Number::Clamp(NUMBER_TO_CLAMP, MIN_VALUE, MAX_VALUE);
        REQUIRE(NUMBER_TO_CLAMP == result);
    }
}
