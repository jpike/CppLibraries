#pragma once

#include "Math/Vector2.h"

/// A namespace for testing the code in the corresponding class.
namespace VECTOR_2_TESTS
{
    TEST_CASE("A vector can be scaled.", "[Vector2]")
    {
        constexpr unsigned int SCALE_FACTOR = 3;
        MATH::Vector2ui vector(2, 4);
        MATH::Vector2ui scaled_vector = MATH::Vector2ui::Scale(SCALE_FACTOR, vector);
        REQUIRE(6 == scaled_vector.X);
        REQUIRE(12 == scaled_vector.Y);
    }

    TEST_CASE("A vector can be normalized to unit length.", "[Vector2]")
    {
        MATH::Vector2f original_vector(2.0f, 5.0f);
        MATH::Vector2f normalized_vector = MATH::Vector2f::Normalize(original_vector);

        REQUIRE(Approx(0.37139f) == normalized_vector.X);
        REQUIRE(Approx(0.92848f) == normalized_vector.Y);

        float normalized_vector_length = normalized_vector.Length();
        REQUIRE(1.0f == normalized_vector_length);
    }

    TEST_CASE("The dot product can be computed.", "[Vector2]")
    {
        MATH::Vector2ui vector_1(1, 2);
        MATH::Vector2ui vector_2(3, 4);

        unsigned int actual_dot_product = MATH::Vector2ui::DotProduct(vector_1, vector_2);

        REQUIRE(11 == actual_dot_product);
    }

    TEST_CASE("Equal vectors are equal.", "[Vector2]")
    {
        MATH::Vector2ui vector_1(1, 2);
        MATH::Vector2ui vector_2(1, 2);

        bool vectors_equal = (vector_1 == vector_2);
        REQUIRE(vectors_equal);

        bool vectors_unequal = (vector_1 != vector_2);
        REQUIRE(!vectors_unequal);
    }

    TEST_CASE("Unqqual vectors are not equal.", "[Vector2]")
    {
        MATH::Vector2ui vector_1(1, 2);
        MATH::Vector2ui vector_2(3, 4);

        bool vectors_equal = (vector_1 == vector_2);
        REQUIRE(!vectors_equal);

        bool vectors_unequal = (vector_1 != vector_2);
        REQUIRE(vectors_unequal);
    }

    TEST_CASE("Vectors can be added.", "[Vector2]")
    {
        MATH::Vector2ui vector_1(1, 2);
        MATH::Vector2ui vector_2(3, 4);

        MATH::Vector2ui sum = vector_1 + vector_2;
        REQUIRE(4 == sum.X);
        REQUIRE(6 == sum.Y);

        vector_1 += vector_2;
        REQUIRE(4 == vector_1.X);
        REQUIRE(6 == vector_1.Y);
    }

    TEST_CASE("Vectors can be subtracted.", "[Vector2]")
    {
        MATH::Vector2ui smaller_vector(1, 2);
        MATH::Vector2ui larger_vector(3, 5);

        MATH::Vector2ui difference = larger_vector - smaller_vector;
        REQUIRE(2 == difference.X);
        REQUIRE(3 == difference.Y);
    }

    TEST_CASE("The length can be computed.", "[Vector2]")
    {
        MATH::Vector2f original_vector(2.0f, 5.0f);
        float actual_length = original_vector.Length();
        REQUIRE(Approx(5.38516f) == actual_length);
    }
}
