#include <limits>
#include <catch.hpp>
#include "Graphics/Color.h"

TEST_CASE("A 32-bit packed RGBA color can be unpacked.", "[Color][Unpack][RGBA]")
{
    // DEFINE A 32-BIT COLOR TO UNPACK.
    constexpr uint32_t PACKED_COLOR = 0x12345678;

    // UNPACK THE COLOR AS RGBA.
    GRAPHICS::Color unpacked_color = GRAPHICS::Color::Unpack(PACKED_COLOR, GRAPHICS::ColorFormat::RGBA);

    // VERIFY THE COLOR WAS CORRECTLY UNPACKED.
    constexpr uint8_t EXPECTED_RED = 0x12;
    uint8_t actual_red = unpacked_color.GetRedAsUint8();
    REQUIRE(EXPECTED_RED == actual_red);

    constexpr uint8_t EXPECTED_GREEN = 0x34;
    uint8_t actual_green = unpacked_color.GetGreenAsUint8();
    REQUIRE(EXPECTED_GREEN == actual_green);

    constexpr uint8_t EXPECTED_BLUE = 0x56;
    uint8_t actual_blue = unpacked_color.GetBlueAsUint8();
    REQUIRE(EXPECTED_BLUE == actual_blue);

    constexpr uint8_t EXPECTED_ALPHA = 0x78;
    uint8_t actual_alpha = unpacked_color.GetAlphaAsUint8();
    REQUIRE(EXPECTED_ALPHA == actual_alpha);
}

TEST_CASE("A 32-bit packed ARGB color can be unpacked.", "[Color][Unpack][ARGB]")
{
    // DEFINE A 32-BIT COLOR TO UNPACK.
    constexpr uint32_t PACKED_COLOR = 0x90ABCDEF;

    // UNPACK THE COLOR AS ARGB.
    GRAPHICS::Color unpacked_color = GRAPHICS::Color::Unpack(PACKED_COLOR, GRAPHICS::ColorFormat::ARGB);

    // VERIFY THE COLOR WAS CORRECTLY UNPACKED.
    constexpr uint8_t EXPECTED_RED = 0xAB;
    uint8_t actual_red = unpacked_color.GetRedAsUint8();
    REQUIRE(EXPECTED_RED == actual_red);

    constexpr uint8_t EXPECTED_GREEN = 0xCD;
    uint8_t actual_green = unpacked_color.GetGreenAsUint8();
    REQUIRE(EXPECTED_GREEN == actual_green);

    constexpr uint8_t EXPECTED_BLUE = 0xEF;
    uint8_t actual_blue = unpacked_color.GetBlueAsUint8();
    REQUIRE(EXPECTED_BLUE == actual_blue);

    constexpr uint8_t EXPECTED_ALPHA = 0x90;
    uint8_t actual_alpha = unpacked_color.GetAlphaAsUint8();
    REQUIRE(EXPECTED_ALPHA == actual_alpha);
}

TEST_CASE("A 32-bit packed color in an invalid format is not unpacked.", "[Color][Unpack][Invalid]")
{
    // DEFINE A 32-BIT COLOR TO UNPACK.
    constexpr uint32_t PACKED_COLOR = 0x1F2E3D4C;

    // TRY TO UNPACK THE COLOR WITH AN INVALID FORMAT.
    constexpr GRAPHICS::ColorFormat INVALID_COLOR_FORMAT = static_cast<GRAPHICS::ColorFormat>(std::numeric_limits<int>::max());
    GRAPHICS::Color unpacked_color = GRAPHICS::Color::Unpack(PACKED_COLOR, INVALID_COLOR_FORMAT);

    // VERIFY A DEFAULT COLOR WAS RETURNED INSTEAD.
    REQUIRE(GRAPHICS::Color::BLACK.Red == unpacked_color.Red);
    REQUIRE(GRAPHICS::Color::BLACK.Green == unpacked_color.Green);
    REQUIRE(GRAPHICS::Color::BLACK.Blue == unpacked_color.Blue);
    REQUIRE(GRAPHICS::Color::BLACK.Alpha == unpacked_color.Alpha);
}

TEST_CASE("RGB color components can be scaled.", "[Color][ScaleRedGreenBlue]")
{
    // DEFINE A COLOR TO SCALE.
    const GRAPHICS::Color ORIGINAL_COLOR(0.2f, 0.3f, 0.4f, 0.5f);

    // SCALE THE COLOR.
    constexpr float SCALE_FACTOR = 2.0f;
    GRAPHICS::Color scaled_color = GRAPHICS::Color::ScaleRedGreenBlue(SCALE_FACTOR, ORIGINAL_COLOR);

    // VERIFY THE COLOR WAS PROPERLY SCALED.
    REQUIRE(0.4f == scaled_color.Red);
    REQUIRE(0.6f == scaled_color.Green);
    REQUIRE(0.8f == scaled_color.Blue);
    REQUIRE(0.5f == scaled_color.Alpha);
}

TEST_CASE("Scaling RGB color components keep them clamped to their maximums.", "[Color][ScaleRedGreenBlue]")
{
    // DEFINE A COLOR TO SCALE.
    const GRAPHICS::Color ORIGINAL_COLOR(0.6f, 0.7f, 0.8f, 5.0f);

    // SCALE THE COLOR.
    constexpr float SCALE_FACTOR = 2.0f;
    GRAPHICS::Color scaled_color = GRAPHICS::Color::ScaleRedGreenBlue(SCALE_FACTOR, ORIGINAL_COLOR);

    // VERIFY THE COLOR WAS PROPERLY SCALED.
    REQUIRE(GRAPHICS::Color::MAX_FLOAT_COLOR_COMPONENT == scaled_color.Red);
    REQUIRE(GRAPHICS::Color::MAX_FLOAT_COLOR_COMPONENT == scaled_color.Green);
    REQUIRE(GRAPHICS::Color::MAX_FLOAT_COLOR_COMPONENT == scaled_color.Blue);
    REQUIRE(GRAPHICS::Color::MAX_FLOAT_COLOR_COMPONENT == scaled_color.Alpha);
}

TEST_CASE("RGB color components can be multipled together.", "[Color][ComponentMultiplyRedGreenBlue]")
{
    // DEFINE TWO COLORS TO MULTIPLY TOGETHER.
    const GRAPHICS::Color COLOR_1(0.2f, 0.3f, 0.4f, 0.5f);
    const GRAPHICS::Color COLOR_2(0.4f, 0.2f, 0.1f, 0.5f);

    // MULTIPLY THE COLOR COMPONENTS.
    GRAPHICS::Color multiplied_color = GRAPHICS::Color::ComponentMultiplyRedGreenBlue(COLOR_1, COLOR_2);

    // VERIFY THE COLOR COMPONENTS WERE PROPERLY MULTIPLED.
    REQUIRE(0.08f == Approx(multiplied_color.Red));
    REQUIRE(0.06f == Approx(multiplied_color.Green));
    REQUIRE(0.04f == Approx(multiplied_color.Blue));
    // The alpha component is always set to the equivalent of black.
    REQUIRE(GRAPHICS::Color::BLACK.Alpha == multiplied_color.Alpha);
}

TEST_CASE("RGB color components can be interpolated.", "[Color][InterpolateRedGreenBlue]")
{
    // DEFINE TWO COLORS TO INTERPOLATE BETWEEEN.
    const GRAPHICS::Color START_COLOR(0.1f, 0.2f, 0.3f, 0.5f);
    const GRAPHICS::Color END_COLOR(0.9f, 0.8f, 0.7f, 0.5f);

    // ARBITRARILY INTERPOLATE BETWEEN THE TWO COLORS.
    constexpr float RATIO_TOWARD_END = 0.6f;
    GRAPHICS::Color interpolated_color = GRAPHICS::Color::InterpolateRedGreenBlue(
        START_COLOR,
        END_COLOR,
        RATIO_TOWARD_END);

    // VERIFY THE COLOR COMPONENTS WERE PROPERLY INTERPOLATED.
    REQUIRE(0.58f == Approx(interpolated_color.Red));
    REQUIRE(0.56f == interpolated_color.Green);
    REQUIRE(0.54f == interpolated_color.Blue);
    // The alpha component is always set to the equivalent of the start color.
    REQUIRE(START_COLOR.Alpha == interpolated_color.Alpha);
}

TEST_CASE("A color can be constructed from 8-bit integer components.", "[Color][Constructor][8-Bit]")
{
    // CREATE A COLOR FROM RGBA INTEGRAL COLOR COMPONENTS.
    GRAPHICS::Color actual_color(
        static_cast<uint8_t>(255),
        static_cast<uint8_t>(128),
        static_cast<uint8_t>(0),
        static_cast<uint8_t>(64));

    // VERIFY THE COLOR COMPONENTS WERE PROPERLY SET.
    REQUIRE(1.0f == actual_color.Red);
    REQUIRE(0.50196f == Approx(actual_color.Green));
    REQUIRE(0.0f == actual_color.Blue);
    REQUIRE(0.25098f == Approx(actual_color.Alpha));
}

TEST_CASE("2 colors with all equal components are equal.", "[Color][EqualityOperator]")
{
    // CREATE TWO DUPLICATE COLORS.
    const GRAPHICS::Color COLOR_1(0.2f, 0.3f, 0.4f, 0.5f);
    const GRAPHICS::Color COLOR_2(0.2f, 0.3f, 0.4f, 0.5f);

    // COMPARE THE COLORS FOR EQUALITY.
    REQUIRE(COLOR_1 == COLOR_2);
}

TEST_CASE("2 colors with all unequal components are not equal.", "[Color][EqualityOperator]")
{
    // CREATE TWO DUPLICATE COLORS.
    const GRAPHICS::Color COLOR_1(0.2f, 0.3f, 0.4f, 0.5f);
    const GRAPHICS::Color COLOR_2(0.6f, 0.7f, 0.8f, 0.9f);

    // COMPARE THE COLORS FOR EQUALITY.
    REQUIRE_FALSE(COLOR_1 == COLOR_2);
}

TEST_CASE("2 colors with only differing red components are not equal.", "[Color][EqualityOperator]")
{
    // CREATE TWO COLORS.
    const GRAPHICS::Color COLOR_1(0.2f, 0.3f, 0.4f, 0.5f);
    const GRAPHICS::Color COLOR_2(0.3f, 0.3f, 0.4f, 0.5f);

    // COMPARE THE COLORS FOR EQUALITY.
    REQUIRE_FALSE(COLOR_1 == COLOR_2);
}

TEST_CASE("2 colors with only differing green components are not equal.", "[Color][EqualityOperator]")
{
    // CREATE TWO COLORS.
    const GRAPHICS::Color COLOR_1(0.3f, 0.3f, 0.4f, 0.5f);
    const GRAPHICS::Color COLOR_2(0.3f, 0.4f, 0.4f, 0.5f);

    // COMPARE THE COLORS FOR EQUALITY.
    REQUIRE_FALSE(COLOR_1 == COLOR_2);
}

TEST_CASE("2 colors with only differing blue components are not equal.", "[Color][EqualityOperator]")
{
    // CREATE TWO COLORS.
    const GRAPHICS::Color COLOR_1(0.2f, 0.3f, 0.4f, 0.5f);
    const GRAPHICS::Color COLOR_2(0.2f, 0.3f, 0.5f, 0.5f);

    // COMPARE THE COLORS FOR EQUALITY.
    REQUIRE_FALSE(COLOR_1 == COLOR_2);
}

TEST_CASE("2 colors with only differing alpha components are not equal.", "[Color][EqualityOperator]")
{
    // CREATE TWO COLORS.
    const GRAPHICS::Color COLOR_1(0.2f, 0.3f, 0.4f, 0.5f);
    const GRAPHICS::Color COLOR_2(0.2f, 0.3f, 0.4f, 0.6f);

    // COMPARE THE COLORS FOR EQUALITY.
    REQUIRE_FALSE(COLOR_1 == COLOR_2);
}

TEST_CASE("2 colors can be added.", "[Color][AdditionAssignmentOperator]")
{
    // CREATE TWO COLORS.
    GRAPHICS::Color color_to_be_added_to(0.2f, 0.3f, 0.4f, 0.5f);
    GRAPHICS::Color color_to_be_added(0.1f, 0.2f, 0.3f, 0.4f);

    // ADD THE SECOND COLOR TO THE FIRST.
    GRAPHICS::Color returned_color = (color_to_be_added_to += color_to_be_added);

    // VERIFY THE COLOR COMPONENTS WERE PROPERLY ADDED.
    REQUIRE(0.3f == color_to_be_added_to.Red);
    REQUIRE(0.5f == color_to_be_added_to.Green);
    REQUIRE(0.7f == Approx(color_to_be_added_to.Blue));
    REQUIRE(0.9f == color_to_be_added_to.Alpha);

    // VERIFY THE RETURNED COLOR MATCHES THE COLOR ADDED TO.
    REQUIRE(returned_color == color_to_be_added_to);
}

TEST_CASE("Adding colors has the result clamped.", "[Color][AdditionAssignmentOperator]")
{
    // CREATE TWO COLORS.
    GRAPHICS::Color color_to_be_added_to(0.2f, 0.3f, 0.4f, 0.5f);
    GRAPHICS::Color color_to_be_added(0.9f, 1.0f, 0.8f, 0.6f);

    // ADD THE SECOND COLOR TO THE FIRST.
    GRAPHICS::Color returned_color = (color_to_be_added_to += color_to_be_added);

    // VERIFY THE COLOR COMPONENTS WERE PROPERLY ADDED.
    REQUIRE(1.0f == color_to_be_added_to.Red);
    REQUIRE(1.0f == color_to_be_added_to.Green);
    REQUIRE(1.0f == color_to_be_added_to.Blue);
    REQUIRE(1.0f == color_to_be_added_to.Alpha);

    // VERIFY THE RETURNED COLOR MATCHES THE COLOR ADDED TO.
    REQUIRE(returned_color == color_to_be_added_to);
}

TEST_CASE("A color can be packed to RGBA format.", "[Color][Pack][RGBA]")
{
    // CREATE A COLOR TO PACK;
    const GRAPHICS::Color COLOR_TO_PACK(
        static_cast<uint8_t>(0x12),
        static_cast<uint8_t>(0x34),
        static_cast<uint8_t>(0x56),
        static_cast<uint8_t>(0x78));

    // PACK THE COLOR.
    uint32_t actual_packed_color = COLOR_TO_PACK.Pack(GRAPHICS::ColorFormat::RGBA);

    // VERIFY THE COLOR WAS PROPERLY PACKED.
    constexpr uint32_t EXPECTED_PACKED_COLOR = 0x12345678;
    REQUIRE(EXPECTED_PACKED_COLOR == actual_packed_color);
}

TEST_CASE("A color can be packed to ARGB format.", "[Color][Pack][ARGB]")
{
    // CREATE A COLOR TO PACK.
    const GRAPHICS::Color COLOR_TO_PACK(
        static_cast<uint8_t>(0x90),
        static_cast<uint8_t>(0xAB),
        static_cast<uint8_t>(0xCD),
        static_cast<uint8_t>(0xEF));

    // PACK THE COLOR.
    uint32_t actual_packed_color = COLOR_TO_PACK.Pack(GRAPHICS::ColorFormat::ARGB);

    // VERIFY THE COLOR WAS PROPERLY PACKED.
    constexpr uint32_t EXPECTED_PACKED_COLOR = 0xEF90ABCD;
    REQUIRE(EXPECTED_PACKED_COLOR == actual_packed_color);
}

TEST_CASE("Trying to pack a color to an invalid format results in a default color.", "[Color][Pack][Invalid]")
{
    // CREATE A COLOR TO PACK.
    const GRAPHICS::Color COLOR_TO_PACK(
        static_cast<uint8_t>(0x90),
        static_cast<uint8_t>(0xAB),
        static_cast<uint8_t>(0xCD),
        static_cast<uint8_t>(0xEF));

    // TRY TO PACK THE COLOR TO AN INVALID FORMAT.
    constexpr GRAPHICS::ColorFormat INVALID_COLOR_FORMAT = static_cast<GRAPHICS::ColorFormat>(std::numeric_limits<int>::max());
    uint32_t actual_packed_color = COLOR_TO_PACK.Pack(INVALID_COLOR_FORMAT);

    // VERIFY A DEFAULT COLOR WAS RETURNED.
    REQUIRE(0 == actual_packed_color);
}

TEST_CASE("Negative color components are clamped to zero.", "[Color][Clamp]")
{
    // CREATE A COLOR WITH NEGATIVE COMPONENTS.
    GRAPHICS::Color color(
        -0.1f,
        -1.0f,
        -2.5f,
        -100.0f);

    // CLAMP THE COLOR.
    color.Clamp();

    // VERIFY THE COLOR WAS PROPERLY CLAMPED.
    REQUIRE(GRAPHICS::Color::MIN_FLOAT_COLOR_COMPONENT == color.Red);
    REQUIRE(GRAPHICS::Color::MIN_FLOAT_COLOR_COMPONENT == color.Green);
    REQUIRE(GRAPHICS::Color::MIN_FLOAT_COLOR_COMPONENT == color.Blue);
    REQUIRE(GRAPHICS::Color::MIN_FLOAT_COLOR_COMPONENT == color.Alpha);
}

TEST_CASE("Negative color components are clamped to one.", "[Color][Clamp]")
{
    // CREATE A COLOR WITH NEGATIVE COMPONENTS.
    GRAPHICS::Color color(
        1.1f,
        2.0f,
        3.5f,
        100.0f);

    // CLAMP THE COLOR.
    color.Clamp();

    // VERIFY THE COLOR WAS PROPERLY CLAMPED.
    REQUIRE(GRAPHICS::Color::MAX_FLOAT_COLOR_COMPONENT == color.Red);
    REQUIRE(GRAPHICS::Color::MAX_FLOAT_COLOR_COMPONENT == color.Green);
    REQUIRE(GRAPHICS::Color::MAX_FLOAT_COLOR_COMPONENT == color.Blue);
    REQUIRE(GRAPHICS::Color::MAX_FLOAT_COLOR_COMPONENT == color.Alpha);
}
