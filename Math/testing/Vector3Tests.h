#pragma once

#include "Math/Vector3.h"

/// A namespace for testing the code in the corresponding class.
namespace VECTOR_3_TESTS
{
    TEST_CASE("A 3D vector can be scaled.", "[Vector3]")
    {
        constexpr unsigned int SCALE_FACTOR = 3;
        MATH::Vector3ui vector(2, 4, 5);
        MATH::Vector3ui scaled_vector = MATH::Vector3ui::Scale(SCALE_FACTOR, vector);
        CHECK(6 == scaled_vector.X);
        CHECK(12 == scaled_vector.Y);
        CHECK(15 == scaled_vector.Z);
    }

    TEST_CASE("A 3D vector can be normalized to unit length.", "[Vector3]")
    {
        MATH::Vector3f original_vector(2.0f, 5.0f, 3.0f);
        MATH::Vector3f normalized_vector = MATH::Vector3f::Normalize(original_vector);

        CHECK(Approx(0.32444f) == normalized_vector.X);
        CHECK(Approx(0.81111f) == normalized_vector.Y);
        CHECK(Approx(0.48666f) == normalized_vector.Z);

        float normalized_vector_length = normalized_vector.Length();
        CHECK(1.0f == normalized_vector_length);
    }

    TEST_CASE("The dot product can be computed for a 3D vector.", "[Vector3]")
    {
        MATH::Vector3ui vector_1(1, 2, 3);
        MATH::Vector3ui vector_2(4, 5, 6);

        unsigned int actual_dot_product = MATH::Vector3ui::DotProduct(vector_1, vector_2);

        CHECK(32 == actual_dot_product);
    }

    /// @todo   Cross product tests!

    TEST_CASE("Equal 3D vectors are equal.", "[Vector3]")
    {
        MATH::Vector3ui vector_1(1, 2, 3);
        MATH::Vector3ui vector_2(1, 2, 3);

        bool vectors_equal = (vector_1 == vector_2);
        CHECK(vectors_equal);

        bool vectors_unequal = (vector_1 != vector_2);
        CHECK(!vectors_unequal);
    }

    TEST_CASE("Unequal 3D vectors are not equal.", "[Vector3]")
    {
        MATH::Vector3ui vector_1(1, 2, 3);
        MATH::Vector3ui vector_2(4, 5, 6);

        bool vectors_equal = (vector_1 == vector_2);
        CHECK(!vectors_equal);

        bool vectors_unequal = (vector_1 != vector_2);
        CHECK(vectors_unequal);
    }

    TEST_CASE("3D vectors can be added.", "[Vector3]")
    {
        MATH::Vector3ui vector_1(1, 2, 3);
        MATH::Vector3ui vector_2(4, 5, 6);

        MATH::Vector3ui sum = vector_1 + vector_2;
        CHECK(5 == sum.X);
        CHECK(7 == sum.Y);
        CHECK(9 == sum.Z);

        vector_1 += vector_2;
        CHECK(5 == vector_1.X);
        CHECK(7 == vector_1.Y);
        CHECK(9 == vector_1.Z);
    }

    TEST_CASE("3D vectors can be subtracted.", "[Vector3]")
    {
        MATH::Vector3ui smaller_vector(1, 2, 3);
        MATH::Vector3ui larger_vector(3, 5, 7);

        MATH::Vector3ui difference = larger_vector - smaller_vector;
        CHECK(2 == difference.X);
        CHECK(3 == difference.Y);
        CHECK(4 == difference.Z);
    }

    TEST_CASE("A 3D vector can be negated.", "[Vector3]")
    {
        MATH::Vector3ui original_vector(1, 2, 3);
        MATH::Vector3ui negated_vector = -original_vector;

        CHECK(-1 == negated_vector.X);
        CHECK(-2 == negated_vector.Y);
        CHECK(-3 == negated_vector.Z);
    }

    TEST_CASE("The length can be computed for a 3D vector.", "[Vector3]")
    {
        MATH::Vector3f original_vector(2.0f, 5.0f, 7.0f);
        float actual_length = original_vector.Length();
        CHECK(8.83176f == actual_length);
    }

    TEST_CASE("A string printout of 3D vector components can be obtained.", "[Vector3]")
    {
        MATH::Vector3ui original_vector(2, 5, 7);
        std::string actual_string = original_vector.ToString();

        const std::string EXPECTED_STRING = "(2, 5, 7)";
        CHECK(EXPECTED_STRING == actual_string);
    }
}
