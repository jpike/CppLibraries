#include <catch.hpp>
#include "Graphics/DepthBuffer.h"

TEST_CASE("A newly constructed depth buffer is cleared to the max depth.", "[DepthBuffer][Constructor]")
{
    // CREATE A DEPTH BUFFER.
    constexpr unsigned int WIDTH_IN_PIXELS = 2;
    constexpr unsigned int HEIGHT_IN_PIXELS = 2;
    GRAPHICS::DepthBuffer depth_buffer(WIDTH_IN_PIXELS, HEIGHT_IN_PIXELS);

    // VERIFY THE BUFFER WAS CLEARED TO THE MAX DEPTH.
    for (unsigned int y = 0; y < HEIGHT_IN_PIXELS; ++y)
    {
        for (unsigned int x = 0; x < WIDTH_IN_PIXELS; ++x)
        {
            // VERIFY THE CURRENT DEPTH VALUE IS THE MAX.
            float actual_current_depth = depth_buffer.GetDepth(x, y);
            REQUIRE(GRAPHICS::DepthBuffer::MAX_DEPTH == actual_current_depth);
        }
    }
}

TEST_CASE("A depth buffer can be cleared to a specific depth.", "[DepthBuffer][ClearToDepth]")
{
    // CREATE A DEPTH BUFFER.
    constexpr unsigned int WIDTH_IN_PIXELS = 2;
    constexpr unsigned int HEIGHT_IN_PIXELS = 2;
    GRAPHICS::DepthBuffer depth_buffer(WIDTH_IN_PIXELS, HEIGHT_IN_PIXELS);

    // CLEAR TO A PARTICULAR DEPTH.
    constexpr float ARBITRARY_DEPTH = 0.5f;
    depth_buffer.ClearToDepth(ARBITRARY_DEPTH);

    // VERIFY THE BUFFER WAS CLEARED TO THE SPECIFIED DEPTH.
    for (unsigned int y = 0; y < HEIGHT_IN_PIXELS; ++y)
    {
        for (unsigned int x = 0; x < WIDTH_IN_PIXELS; ++x)
        {
            // VERIFY THE CURRENT DEPTH VALUE IS THE MAX.
            float actual_current_depth = depth_buffer.GetDepth(x, y);
            REQUIRE(ARBITRARY_DEPTH == actual_current_depth);
        }
    }
}

TEST_CASE("A depth cannot be written out-of-range of the depth buffer.", "[DepthBuffer][WriteDepth]")
{
    // CREATE A DEPTH BUFFER.
    constexpr unsigned int WIDTH_IN_PIXELS = 2;
    constexpr unsigned int HEIGHT_IN_PIXELS = 2;
    GRAPHICS::DepthBuffer depth_buffer(WIDTH_IN_PIXELS, HEIGHT_IN_PIXELS);

    // ATTEMPT TO WRITE A DEPTH OUT-OF-RANGE.
    constexpr unsigned int OUT_OF_RANGE_X = WIDTH_IN_PIXELS;
    constexpr unsigned int OUT_OF_RANGE_Y = HEIGHT_IN_PIXELS;
    constexpr float ARBITRARY_DEPTH = 0.5f;
    depth_buffer.WriteDepth(OUT_OF_RANGE_X, OUT_OF_RANGE_Y, ARBITRARY_DEPTH);

    // VERIFY THAT THE DEPTH IS STILL AT ITS MAX.
    for (unsigned int y = 0; y < HEIGHT_IN_PIXELS; ++y)
    {
        for (unsigned int x = 0; x < WIDTH_IN_PIXELS; ++x)
        {
            // VERIFY THE CURRENT DEPTH VALUE IS THE MAX.
            float actual_current_depth = depth_buffer.GetDepth(x, y);
            REQUIRE(GRAPHICS::DepthBuffer::MAX_DEPTH == actual_current_depth);
        }
    }
}

TEST_CASE("A depth value can be written to specific coordinates.", "[DepthBuffer][WriteDepth]")
{
    // CREATE A DEPTH BUFFER.
    constexpr unsigned int WIDTH_IN_PIXELS = 2;
    constexpr unsigned int HEIGHT_IN_PIXELS = 2;
    GRAPHICS::DepthBuffer depth_buffer(WIDTH_IN_PIXELS, HEIGHT_IN_PIXELS);

    // WRITE A DEPTH VALUE INTO THE BUFFER.
    constexpr unsigned int ARBITRARY_X = 0;
    constexpr unsigned int ARBITRARY_Y = 1;
    constexpr float ARBITRARY_DEPTH = 0.5f;
    depth_buffer.WriteDepth(ARBITRARY_X, ARBITRARY_Y, ARBITRARY_DEPTH);

    // VERIFY THE DEPTH VALUE WAS WRITTEN AT THE CORRECT COORDINATES.
    float actual_depth = depth_buffer.GetDepth(ARBITRARY_X, ARBITRARY_Y);
    REQUIRE(ARBITRARY_DEPTH == actual_depth);
}
