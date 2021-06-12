#pragma once

#include <random>

namespace MATH
{
    /// A random number generator.  This class serves to provide
    /// a more convenient interface (compared to the built-in C++
    /// libraries) for certain situations of generating random numbers.
    ///
    /// Many methods are templated to let them adapt to different data types,
    /// but no special logic exists for these types (they're basically just
    /// casted directly to from unsigned integers).
    class RandomNumberGenerator
    {
    public:
        // RANDOM NUMBERS.
        template <typename NumberType>
        NumberType RandomNumber();
        template <typename NumberType>
        NumberType RandomNumberLessThan(const NumberType excluded_max);
        template <typename NumberType>
        NumberType RandomInRange(const NumberType min, const NumberType max);

        // RANDOM ENUMS.
        template <typename EnumType>
        EnumType RandomEnum();

    private:
        /// The underlying random device.
        std::random_device RandomDevice;
    };

    /// Generates a random number of the specified type.
    /// @return A random number.
    template <typename NumberType>
    NumberType RandomNumberGenerator::RandomNumber()
    {
        unsigned int random_number = RandomDevice();
        return static_cast<NumberType>(random_number);
    }

    /// Generates a random number less than the specified value.
    /// @param[in]  excluded_max - The number for which to generate a value less than.
    /// @return A random number between [0, excluded_max).
    template <typename NumberType>
    NumberType RandomNumberGenerator::RandomNumberLessThan(const NumberType excluded_max)
    {
        unsigned int random_number = RandomNumber<unsigned int>();
        unsigned int random_number_less_than = random_number % static_cast<unsigned int>(excluded_max);
        return static_cast<NumberType>(random_number_less_than);
    }

    /// Generates a random number within the specified range.
    /// @param[in]  min - The minimum possible random number to generate.
    /// @param[in]  max - The maximum possible random number to generate.
    /// @return A random number between [min, max].
    template <typename NumberType>
    NumberType RandomNumberGenerator::RandomInRange(const NumberType min, const NumberType max)
    {
        NumberType number_count_in_range = max - min + 1;
        NumberType random_number = RandomNumberLessThan(number_count_in_range);
        NumberType random_number_in_range = min + random_number;
        return random_number_in_range;
    }

    /// Generates a random enum value.
    /// The enum type is expected to have a COUNT value indicating the
    /// number of different enum values, and this COUNT value is excluded
    /// from the generated range.
    /// @return A random enum value of the specified type.
    template <typename EnumType>
    EnumType RandomNumberGenerator::RandomEnum()
    {
        const unsigned int ENUM_VALUE_COUNT = static_cast<unsigned int>(EnumType::COUNT);
        unsigned int random_number = RandomNumberLessThan(ENUM_VALUE_COUNT);
        EnumType random_enum_value = static_cast<EnumType>(random_number);
        return random_enum_value;
    }
}
