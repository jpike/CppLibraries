#pragma once

#include <memory>
#include <stdexcept>
#include "Containers/Array2D.h"

/// A namespace for testing the Array2D class.
namespace ARRAY_2D_TESTS
{
    TEST_CASE("Array dimensions properly set upon construction.", "[Array2D]")
    {
        // CREATE A 2D ARRAY.
        const unsigned int WIDTH = 4;
        const unsigned int HEIGHT = 3;
        CONTAINERS::Array2D<int> array_2d(WIDTH, HEIGHT);

        // VALIDATE THE DIMENSIONS.
        unsigned int actual_width = array_2d.GetWidth();
        REQUIRE(WIDTH == actual_width);

        unsigned int actual_height = array_2d.GetHeight();
        REQUIRE(HEIGHT == actual_height);
    }

    TEST_CASE("An array can be constructed with initial data.", "[Array2D]")
    {
        // CREATE A 2D ARRAY.
        const unsigned int WIDTH = 3;
        const unsigned int HEIGHT = 2;
        CONTAINERS::Array2D<int> array_2d(WIDTH, HEIGHT, {
            1, 2, 3,
            4, 5, 6 } );

        // VALIDATE THE DIMENSIONS.
        unsigned int actual_width = array_2d.GetWidth();
        REQUIRE(WIDTH == actual_width);

        unsigned int actual_height = array_2d.GetHeight();
        REQUIRE(HEIGHT == actual_height);

        // VALIDATE THE DATA.
        REQUIRE(1 == array_2d(0, 0));
        REQUIRE(2 == array_2d(1, 0));
        REQUIRE(3 == array_2d(2, 0));
        REQUIRE(4 == array_2d(0, 1));
        REQUIRE(5 == array_2d(1, 1));
        REQUIRE(6 == array_2d(2, 1));
    }

    TEST_CASE("An exception is thrown if an attempt is made to construct an array with insufficient initial data.", "[Array2D]")
    {
        bool exception_thrown = false;
        try
        {
            // CREATE A 2D ARRAY WITHOUT ENOUGH INITIAL DATA.
            const unsigned int WIDTH = 3;
            const unsigned int HEIGHT = 2;
            CONTAINERS::Array2D<int> array_2d(WIDTH, HEIGHT, {
                1, 2, 3,
                4, 5 });
        }
        catch (const std::exception&)
        {
            exception_thrown = true;
        }
        
        // VALIDATE AN EXCEPTION WAS THROWN.
        REQUIRE(exception_thrown);
    }

    TEST_CASE("Two arrays may be equal.", "[Array2D]")
    {
        // CREATE TWO EQUAL ARRAYS.
        const unsigned int WIDTH = 4;
        const unsigned int HEIGHT = 3;
        CONTAINERS::Array2D<int> first_array_2d(WIDTH, HEIGHT);
        CONTAINERS::Array2D<int> second_array_2d(WIDTH, HEIGHT);

        // MAKE SURE THE ARRAYS ARE EQUAL.
        bool arrays_equal = (first_array_2d == second_array_2d);
        REQUIRE(arrays_equal);

        bool arrays_unequal = (first_array_2d != second_array_2d);
        REQUIRE(!arrays_unequal);
    }

    TEST_CASE("Arrays with different widths aren't equal.", "[Array2D]")
    {
        // CREATE TWO ARRAYS WITH DIFFERENT WIDTHS.
        const unsigned int FIRST_WIDTH = 4;
        const unsigned int HEIGHT = 3;
        CONTAINERS::Array2D<int> first_array_2d(FIRST_WIDTH, HEIGHT);

        const unsigned int SECOND_WIDTH = 5;
        CONTAINERS::Array2D<int> second_array_2d(SECOND_WIDTH, HEIGHT);

        // MAKE SURE THE ARRAYS AREN'T EQUAL.
        bool arrays_equal = (first_array_2d == second_array_2d);
        REQUIRE(!arrays_equal);

        bool arrays_unequal = (first_array_2d != second_array_2d);
        REQUIRE(arrays_unequal);
    }

    TEST_CASE("Arrays with different heights aren't equal.", "[Array2D]")
    {
        // CREATE TWO ARRAYS WITH DIFFERENT HEIGHTS.
        const unsigned int WIDTH = 4;
        const unsigned int FIRST_HEIGHT = 3;
        CONTAINERS::Array2D<int> first_array_2d(WIDTH, FIRST_HEIGHT);

        const unsigned int SECOND_HEIGHT = 6;
        CONTAINERS::Array2D<int> second_array_2d(WIDTH, SECOND_HEIGHT);

        // MAKE SURE THE ARRAYS AREN'T EQUAL.
        bool arrays_equal = (first_array_2d == second_array_2d);
        REQUIRE(!arrays_equal);

        bool arrays_unequal = (first_array_2d != second_array_2d);
        REQUIRE(arrays_unequal);
    }

    TEST_CASE("Arrays with different data aren't equal.", "[Array2D]")
    {
        // CREATE TWO ARRAYS WITH DIFFERENT DATA.
        const unsigned int WIDTH = 4;
        const unsigned int HEIGHT = 3;
        CONTAINERS::Array2D<int> first_array_2d(WIDTH, HEIGHT);
        CONTAINERS::Array2D<int> second_array_2d(WIDTH, HEIGHT);

        const unsigned int ELEMENT_X = 0;
        const unsigned int ELEMENT_Y = 0;
        first_array_2d(ELEMENT_X, ELEMENT_Y) = 7;
        second_array_2d(ELEMENT_X, ELEMENT_Y) = 8;

        // MAKE SURE THE ARRAYS AREN'T EQUAL.
        bool arrays_equal = (first_array_2d == second_array_2d);
        REQUIRE(!arrays_equal);

        bool arrays_unequal = (first_array_2d != second_array_2d);
        REQUIRE(arrays_unequal);
    }

    TEST_CASE("An array can be resized.", "[Array2D]")
    {
        // CREATE A 2D ARRAY.
        const unsigned int ORIGINAL_WIDTH = 4;
        const unsigned int ORIGINAL_HEIGHT = 3;
        CONTAINERS::Array2D<int> array_2d(ORIGINAL_WIDTH, ORIGINAL_HEIGHT);

        // RESIZE THE ARRAY.
        const unsigned int NEW_WIDTH = 24;
        const unsigned int NEW_HEIGHT = 37;
        array_2d.Resize(NEW_WIDTH, NEW_HEIGHT);

        // VALIDATE THE DIMENSIONS.
        unsigned int actual_width = array_2d.GetWidth();
        REQUIRE(NEW_WIDTH == actual_width);

        unsigned int actual_height = array_2d.GetHeight();
        REQUIRE(NEW_HEIGHT == actual_height);
    }

    TEST_CASE("An array with elements that can only be moved can be resized.", "[Array2D]")
    {
        // CREATE A 2D ARRAY.
        const unsigned int ORIGINAL_WIDTH = 4;
        const unsigned int ORIGINAL_HEIGHT = 3;
        CONTAINERS::Array2D< std::unique_ptr<int> > array_2d(ORIGINAL_WIDTH, ORIGINAL_HEIGHT);

        // RESIZE THE ARRAY.
        const unsigned int NEW_WIDTH = 24;
        const unsigned int NEW_HEIGHT = 37;
        array_2d.Resize(NEW_WIDTH, NEW_HEIGHT);

        // VALIDATE THE DIMENSIONS.
        unsigned int actual_width = array_2d.GetWidth();
        REQUIRE(NEW_WIDTH == actual_width);

        unsigned int actual_height = array_2d.GetHeight();
        REQUIRE(NEW_HEIGHT == actual_height);
    }

    TEST_CASE("An element's value can be modified.", "[Array2D]")
    {
        // CREATE A 2D ARRAY.
        const unsigned int WIDTH = 4;
        const unsigned int HEIGHT = 3;
        CONTAINERS::Array2D<int> array_2d(WIDTH, HEIGHT);

        // SET AN ELEMENT.
        const int ELEMENT_X = 2;
        const int ELEMENT_Y = 1;
        const int ELEMENT_VALUE = 72;
        array_2d(ELEMENT_X, ELEMENT_Y) = ELEMENT_VALUE;

        // VALIDATE THAT THE ELEMENT WAS PROPERLY SET.
        int actual_element = array_2d(ELEMENT_X, ELEMENT_Y);
        REQUIRE(ELEMENT_VALUE == actual_element);
    }

    TEST_CASE("Minimum 2D coordinates are valid.", "[Array2D]")
    {
        // CREATE A 2D ARRAY.
        const unsigned int WIDTH = 4;
        const unsigned int HEIGHT = 3;
        CONTAINERS::Array2D<int> array_2d(WIDTH, HEIGHT);

        // SET THE ELEMENT AT THE MINIMUM COORDINATES.
        const unsigned int MIN_X = 0;
        const unsigned int MIN_Y = 0;
        const int ELEMENT_VALUE = 72;
        array_2d(MIN_X, MIN_Y) = ELEMENT_VALUE;

        // VALIDATE THE ELEMENT.
        int actual_element = array_2d(MIN_X, MIN_Y);
        REQUIRE(ELEMENT_VALUE == actual_element);
    }

    TEST_CASE("Maximum 2D coordinates are valid.", "[Array2D]")
    {
        // CREATE A 2D ARRAY.
        const unsigned int WIDTH = 4;
        const unsigned int HEIGHT = 3;
        CONTAINERS::Array2D<int> array_2d(WIDTH, HEIGHT);

        // SET THE ELEMENT AT THE MAXIMUM COORDINATES.
        const unsigned int MAX_X = WIDTH - 1;
        const unsigned int MAX_Y = HEIGHT - 1;
        const int ELEMENT_VALUE = 72;
        array_2d(MAX_X, MAX_Y) = ELEMENT_VALUE;

        // VALIDATE THE ELEMENT.
        int actual_element = array_2d(MAX_X, MAX_Y);
        REQUIRE(ELEMENT_VALUE == actual_element);
    }

    TEST_CASE("Coordinates at the beginning of a row are valid.", "[Array2D]")
    {
        // CREATE A 2D ARRAY.
        const unsigned int WIDTH = 4;
        const unsigned int HEIGHT = 3;
        CONTAINERS::Array2D<int> array_2d(WIDTH, HEIGHT);

        // SET THE ELEMENT AT THE BEGINNING OF THE SECOND ROW.
        const unsigned int MIN_X = 0;
        const unsigned int ELEMENT_Y = 1;
        const int ELEMENT_VALUE = 72;
        array_2d(MIN_X, ELEMENT_Y) = ELEMENT_VALUE;

        // VALIDATE THE ELEMENT.
        int actual_element = array_2d(MIN_X, ELEMENT_Y);
        REQUIRE(ELEMENT_VALUE == actual_element);
    }

    TEST_CASE("Coordinates at the end of a row are valid.", "[Array2D]")
    {
        // CREATE A 2D ARRAY.
        const unsigned int WIDTH = 4;
        const unsigned int HEIGHT = 3;
        CONTAINERS::Array2D<int> array_2d(WIDTH, HEIGHT);

        // SET THE ELEMENT AT THE END OF THE FIRST ROW.
        const unsigned int MAX_X = WIDTH - 1;
        const unsigned int ELEMENT_Y = 0;
        const int ELEMENT_VALUE = 72;
        array_2d(MAX_X, ELEMENT_Y) = ELEMENT_VALUE;

        // VALIDATE THE ELEMENT.
        int actual_element = array_2d(MAX_X, ELEMENT_Y);
        REQUIRE(ELEMENT_VALUE == actual_element);
    }

    TEST_CASE("An x-coordinate outside the array bounds results in an exception.", "[Array2D]")
    {
        // CREATE A 2D ARRAY.
        const unsigned int WIDTH = 4;
        const unsigned int HEIGHT = 3;
        CONTAINERS::Array2D<int> array_2d(WIDTH, HEIGHT);

        // TRY ACCESSING AN ELEMENT OUTSIDE OF THE ARRAY'S HORIZONTAL BOUNDS.
        bool exception_thrown = false;
        try
        {
            const unsigned int ELEMENT_X = WIDTH;
            const unsigned int ELEMENT_Y = 0;
            array_2d(ELEMENT_X, ELEMENT_Y);
        }
        catch (const std::out_of_range&)
        {
            exception_thrown = true;
        }

        // VALIDATE THAT AN EXCEPTION WAS THROWN.
        REQUIRE(exception_thrown);
    }

    TEST_CASE("A y-coordinate outside the array bounds results in an exception.", "[Array2D]")
    {
        // CREATE A 2D ARRAY.
        const unsigned int WIDTH = 4;
        const unsigned int HEIGHT = 3;
        CONTAINERS::Array2D<int> array_2d(WIDTH, HEIGHT);

        // TRY ACCESSING AN ELEMENT OUTSIDE OF THE ARRAY'S VERTICAL BOUNDS.
        bool exception_thrown = false;
        try
        {
            const unsigned int ELEMENT_X = 0;
            const unsigned int ELEMENT_Y = HEIGHT;
            array_2d(ELEMENT_X, ELEMENT_Y);
        }
        catch (const std::out_of_range&)
        {
            exception_thrown = true;
        }

        // VALIDATE THAT AN EXCEPTION WAS THROWN.
        REQUIRE(exception_thrown);
    }
}