#pragma once

#include "Math/Angle.h"

/// A namespace for testing the code in the corresponding class.
namespace ANGLE_TESTS
{
    TEST_CASE("Equal radian values are equal.", "[Angle][Radians]")
    {
        MATH::Angle<float>::Radians radians_1(1.0f);
        MATH::Angle<float>::Radians radians_2(1.0f);
        bool radians_equal = (radians_1 == radians_2);
        REQUIRE(radians_equal);
    }

    TEST_CASE("Unequal radian values are not equal.", "[Angle][Radians]")
    {
        MATH::Angle<float>::Radians radians_1(1.0f);
        MATH::Angle<float>::Radians radians_2(2.0f);
        bool radians_equal = (radians_1 == radians_2);
        REQUIRE(!radians_equal);
    }

    TEST_CASE("Radians can be added.", "[Angle][Radians]")
    {
        MATH::Angle<float>::Radians radians_1(1.0f);
        MATH::Angle<float>::Radians radians_2(2.0f);
        MATH::Angle<float>::Radians sum = radians_1 + radians_2;
        REQUIRE(3.0f == sum.Value);
    }

    TEST_CASE("Radians can be subtracted.", "[Angle][Radians]")
    {
        MATH::Angle<float>::Radians radians_1(1.0f);
        MATH::Angle<float>::Radians radians_2(2.0f);
        MATH::Angle<float>::Radians difference = radians_1 - radians_2;
        REQUIRE(-1.0f == difference.Value);
    }

    TEST_CASE("Radians can be multiplied.", "[Angle][Radians]")
    {
        MATH::Angle<float>::Radians radians_1(2.0f);
        MATH::Angle<float>::Radians radians_2(3.0f);
        MATH::Angle<float>::Radians product = radians_1 * radians_2;
        REQUIRE(6.0f == product.Value);
    }

    TEST_CASE("Radians can be divided.", "[Angle][Radians]")
    {
        MATH::Angle<float>::Radians radians_1(6.0f);
        MATH::Angle<float>::Radians radians_2(2.0f);
        MATH::Angle<float>::Radians quotient = radians_1 / radians_2;
        REQUIRE(3.0f == quotient.Value);
    }

    TEST_CASE("Equal degree values are equal.", "[Angle][Degrees]")
    {
        MATH::Angle<float>::Degrees degrees_1(1.0f);
        MATH::Angle<float>::Degrees degrees_2(1.0f);
        bool degrees_equal = (degrees_1 == degrees_2);
        REQUIRE(degrees_equal);
    }

    TEST_CASE("Unequal degree values are not equal.", "[Angle][Degrees]")
    {
        MATH::Angle<float>::Degrees degrees_1(1.0f);
        MATH::Angle<float>::Degrees degrees_2(2.0f);
        bool degrees_equal = (degrees_1 == degrees_2);
        REQUIRE(!degrees_equal);
    }

    TEST_CASE("Degrees can be added.", "[Angle][Degrees]")
    {
        MATH::Angle<float>::Degrees degrees_1(1.0f);
        MATH::Angle<float>::Degrees degrees_2(2.0f);
        MATH::Angle<float>::Degrees sum = degrees_1 + degrees_2;
        REQUIRE(3.0f == sum.Value);
    }

    TEST_CASE("Degrees can be subtracted.", "[Angle][Degrees]")
    {
        MATH::Angle<float>::Degrees degrees_1(1.0f);
        MATH::Angle<float>::Degrees degrees_2(2.0f);
        MATH::Angle<float>::Degrees difference = degrees_1 - degrees_2;
        REQUIRE(-1.0f == difference.Value);
    }

    TEST_CASE("Degrees can be multiplied.", "[Angle][Degrees]")
    {
        MATH::Angle<float>::Degrees degrees_1(2.0f);
        MATH::Angle<float>::Degrees degrees_2(3.0f);
        MATH::Angle<float>::Degrees product = degrees_1 * degrees_2;
        REQUIRE(6.0f == product.Value);
    }

    TEST_CASE("Degrees can be divided.", "[Angle][Degrees]")
    {
        MATH::Angle<float>::Degrees degrees_1(6.0f);
        MATH::Angle<float>::Degrees degrees_2(2.0f);
        MATH::Angle<float>::Degrees quotient = degrees_1 / degrees_2;
        REQUIRE(3.0f == quotient.Value);
    }

    TEST_CASE("Degrees can be converted to radians.", "[Angle]")
    {
        // The entire common unit circle divisions are tested.
        MATH::Angle<float>::Degrees degrees_0(0.0f);
        MATH::Angle<float>::Radians radians_0 = MATH::Angle<float>::DegreesToRadians(degrees_0);
        REQUIRE(0.0f == radians_0.Value);

        constexpr float PI = 3.14159f;
        MATH::Angle<float>::Degrees degrees_30(30.0f);
        MATH::Angle<float>::Radians radians_pi_over_6 = MATH::Angle<float>::DegreesToRadians(degrees_30);
        REQUIRE((PI / 6.0f) == radians_pi_over_6.Value);

        MATH::Angle<float>::Degrees degrees_45(45.0f);
        MATH::Angle<float>::Radians radians_pi_over_4 = MATH::Angle<float>::DegreesToRadians(degrees_45);
        REQUIRE((PI / 4.0f) == radians_pi_over_4.Value);

        MATH::Angle<float>::Degrees degrees_60(60.0f);
        MATH::Angle<float>::Radians radians_pi_over_3 = MATH::Angle<float>::DegreesToRadians(degrees_60);
        REQUIRE((PI / 3.0f) == radians_pi_over_3.Value);

        MATH::Angle<float>::Degrees degrees_90(90.0f);
        MATH::Angle<float>::Radians radians_pi_over_2 = MATH::Angle<float>::DegreesToRadians(degrees_90);
        REQUIRE((PI / 2.0f) == radians_pi_over_2.Value);

        MATH::Angle<float>::Degrees degrees_120(120.0f);
        MATH::Angle<float>::Radians radians_2_pi_over_3 = MATH::Angle<float>::DegreesToRadians(degrees_120);
        REQUIRE((2.0f * PI / 3.0f) == radians_2_pi_over_3.Value);

        MATH::Angle<float>::Degrees degrees_135(135.0f);
        MATH::Angle<float>::Radians radians_3_pi_over_4 = MATH::Angle<float>::DegreesToRadians(degrees_135);
        REQUIRE((3.0f * PI / 4.0f) == radians_3_pi_over_4.Value);

        MATH::Angle<float>::Degrees degrees_150(150.0f);
        MATH::Angle<float>::Radians radians_5_pi_over_6 = MATH::Angle<float>::DegreesToRadians(degrees_150);
        REQUIRE(Approx(5.0f * PI / 6.0f) == radians_5_pi_over_6.Value);

        MATH::Angle<float>::Degrees degrees_180(180.0f);
        MATH::Angle<float>::Radians radians_pi = MATH::Angle<float>::DegreesToRadians(degrees_180);
        REQUIRE(PI == radians_pi.Value);

        MATH::Angle<float>::Degrees degrees_210(210.0f);
        MATH::Angle<float>::Radians radians_7_pi_over_6 = MATH::Angle<float>::DegreesToRadians(degrees_210);
        REQUIRE((7.0f * PI / 6.0f) == radians_7_pi_over_6.Value);

        MATH::Angle<float>::Degrees degrees_225(225.0f);
        MATH::Angle<float>::Radians radians_5_pi_over_4 = MATH::Angle<float>::DegreesToRadians(degrees_225);
        REQUIRE((5.0f * PI / 4.0f) == radians_5_pi_over_4.Value);

        MATH::Angle<float>::Degrees degrees_240(240.0f);
        MATH::Angle<float>::Radians radians_4_pi_over_3 = MATH::Angle<float>::DegreesToRadians(degrees_240);
        REQUIRE((4.0f * PI / 3.0f) == radians_4_pi_over_3.Value);

        MATH::Angle<float>::Degrees degrees_270(270.0f);
        MATH::Angle<float>::Radians radians_3_pi_over_2 = MATH::Angle<float>::DegreesToRadians(degrees_270);
        REQUIRE((3.0f * PI / 2.0f) == radians_3_pi_over_2.Value);

        MATH::Angle<float>::Degrees degrees_300(300.0f);
        MATH::Angle<float>::Radians radians_5_pi_over_3 = MATH::Angle<float>::DegreesToRadians(degrees_300);
        REQUIRE(Approx(5.0f * PI / 3.0f) == radians_5_pi_over_3.Value);

        MATH::Angle<float>::Degrees degrees_315(315.0f);
        MATH::Angle<float>::Radians radians_7_pi_over_4 = MATH::Angle<float>::DegreesToRadians(degrees_315);
        REQUIRE((7.0f * PI / 4.0f) == radians_7_pi_over_4.Value);

        MATH::Angle<float>::Degrees degrees_330(330.0f);
        MATH::Angle<float>::Radians radians_11_pi_over_6 = MATH::Angle<float>::DegreesToRadians(degrees_330);
        REQUIRE((11.0f * PI / 6.0f) == radians_11_pi_over_6.Value);

        MATH::Angle<float>::Degrees degrees_360(360.0f);
        MATH::Angle<float>::Radians radians_2_pi = MATH::Angle<float>::DegreesToRadians(degrees_360);
        REQUIRE((2.0f * PI) == radians_2_pi.Value);
    }
}
