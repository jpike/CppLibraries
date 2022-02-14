#pragma once

#include "Math/Vector4.h"

/// A namespace for testing the code in the corresponding class.
namespace VECTOR_4_TESTS
{
    TEST_CASE("A 4D vector can be scaled.", "[Vector4]")
    {
        constexpr float SCALE_FACTOR = 3.0f;
        MATH::Vector4f vector(2.0f, 4.0f, 5.0f, 6.0f);
        MATH::Vector4f scaled_vector = MATH::Vector4f::Scale(SCALE_FACTOR, vector);
        CHECK(6.0f == scaled_vector.X);
        CHECK(12.0f == scaled_vector.Y);
        CHECK(15.0f == scaled_vector.Z);
        CHECK(18.0f == scaled_vector.W);
    }

    TEST_CASE("A 4D vector can be normalized to unit length.", "[Vector4]")
    {
        MATH::Vector4f original_vector(2.0f, 5.0f, 3.0f, 4.0f);
        MATH::Vector4f normalized_vector = MATH::Vector4f::Normalize(original_vector);

        CHECK(Approx(0.272165537f) == normalized_vector.X);
        CHECK(Approx(0.68041f) == normalized_vector.Y);
        CHECK(Approx(0.40825f) == normalized_vector.Z);
        CHECK(Approx(0.54433f) == normalized_vector.W);

        float normalized_vector_length = normalized_vector.Length();
        CHECK(1.0f == normalized_vector_length);
    }

    TEST_CASE("The dot product can be computed for a 4D vector.", "[Vector4]")
    {
        MATH::Vector4f vector_1(1.0f, 2.0f, 3.0f, 4.0f);
        MATH::Vector4f vector_2(5.0f, 6.0f, 7.0f, 8.0f);

        float actual_dot_product = MATH::Vector4f::DotProduct(vector_1, vector_2);

        CHECK(70.0f == actual_dot_product);
    }

    TEST_CASE("A homogenous 4D position vector can be created from a 3D vector.", "[Vector4]")
    {
        MATH::Vector3f vector_3(2.0f, 3.0f, 4.0f);
        MATH::Vector4f actual_homogenous_vector_4 = MATH::Vector4f::HomogeneousPositionVector(vector_3);
        CHECK(2.0f == actual_homogenous_vector_4.X);
        CHECK(3.0f == actual_homogenous_vector_4.Y);
        CHECK(4.0f == actual_homogenous_vector_4.Z);
        CHECK(1.0f == actual_homogenous_vector_4.W);
    }

    TEST_CASE("Equal 4D vectors are equal.", "[Vector4]")
    {
        MATH::Vector4f vector_1(1.0f, 2.0f, 3.0f, 4.0f);
        MATH::Vector4f vector_2(1.0f, 2.0f, 3.0f, 4.0f);

        bool vectors_equal = (vector_1 == vector_2);
        CHECK(vectors_equal);

        bool vectors_unequal = (vector_1 != vector_2);
        CHECK(!vectors_unequal);
    }

    TEST_CASE("Unequal 4D vectors are not equal.", "[Vector4]")
    {
        MATH::Vector4f vector_1(1.0f, 2.0f, 3.0f, 4.0f);
        MATH::Vector4f vector_2(5.0f, 6.0f, 7.0f, 8.0f);

        bool vectors_equal = (vector_1 == vector_2);
        CHECK(!vectors_equal);

        bool vectors_unequal = (vector_1 != vector_2);
        CHECK(vectors_unequal);
    }

    TEST_CASE("4D vectors can be added.", "[Vector4]")
    {
        MATH::Vector4f vector_1(1.0f, 2.0f, 3.0f, 4.0f);
        MATH::Vector4f vector_2(5.0f, 6.0f, 7.0f, 8.0f);

        MATH::Vector4f sum = vector_1 + vector_2;
        CHECK(6.0f == sum.X);
        CHECK(8.0f == sum.Y);
        CHECK(10.0f == sum.Z);
        CHECK(12.0f == sum.W);

        vector_1 += vector_2;
        CHECK(6.0f == vector_1.X);
        CHECK(8.0f == vector_1.Y);
        CHECK(10.0f == vector_1.Z);
        CHECK(12.0f == vector_1.W);
    }

    TEST_CASE("4D vectors can be subtracted.", "[Vector4]")
    {
        MATH::Vector4f smaller_vector(1.0f, 2.0f, 3.0f, 4.0f);
        MATH::Vector4f larger_vector(3.0f, 5.0f, 7.0f, 9.0f);

        MATH::Vector4f difference = larger_vector - smaller_vector;
        CHECK(2.0f == difference.X);
        CHECK(3.0f == difference.Y);
        CHECK(4.0f == difference.Z);
        CHECK(5.0f == difference.W);
    }

    TEST_CASE("A 4D vector can be negated.", "[Vector4]")
    {
        MATH::Vector4f original_vector(1.0f, 2.0f, 3.0f, 4.0f);
        MATH::Vector4f negated_vector = -original_vector;

        CHECK(-1.0f == negated_vector.X);
        CHECK(-2.0f == negated_vector.Y);
        CHECK(-3.0f == negated_vector.Z);
        CHECK(-4.0f == negated_vector.W);
    }

    TEST_CASE("The length can be computed for a 4D vector.", "[Vector4]")
    {
        MATH::Vector4f original_vector(2.0f, 5.0f, 7.0f, 4.0f);
        float actual_length = original_vector.Length();
        CHECK(9.69536f == actual_length);
    }
}
