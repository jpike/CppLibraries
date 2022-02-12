#pragma once

#include <chrono>
#include <iostream>
#include <string>

/// Holds code related to debugging.
namespace DEBUGGING
{
    /// A timer that can be used for taking timing measurements of code.
    /// @tparam ClockType - The type of clock to use for timing.  Should be std::chrono-compatible.
    template<typename ClockType = std::chrono::high_resolution_clock>
    class Timer
    {
    public:
        /// Starts a timer with the specified name.
        /// @param[in]  name - The name to associate with the timer.
        explicit Timer(const std::string& name) :
            Name(name),
            StartTime(ClockType::now())
        {}

        /// Destructor to automatically print out the elapsed time.
        ~Timer()
        {
            std::cout << GetElapsedTimeText() << std::endl;
        }

        /// Gets text with information about the elapsed time for the timer at the current time.
        /// @return Text describing the current elapsed time of the timer.
        std::string GetElapsedTimeText() const
        {
            auto current_time = ClockType::now();
            auto elapsed_time = current_time - StartTime;
            // Both "raw" and "familiar" timing units are printed out for greater debuggability.
            std::string elapsed_time_text = (
                Name + ": " +
                std::to_string(elapsed_time.count()) + "\t" +
                std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_time).count()) + "ms\t" +
                std::to_string(std::chrono::duration_cast<std::chrono::seconds>(elapsed_time).count()) + "s");
            return elapsed_time_text;
        }

        /// The name associated with the timer.
        std::string Name = {};
        /// The time the timer was started.
        std::chrono::time_point<ClockType> StartTime = {};
    };

    /// A timer with the highest resolution provided by the C++ standard library.
    using HighResolutionTimer = Timer<std::chrono::high_resolution_clock>;
    /// A timer using the system's "wall clock" (familiar timing for humans).
    using SystemClockTimer = Timer<std::chrono::system_clock>;
}
