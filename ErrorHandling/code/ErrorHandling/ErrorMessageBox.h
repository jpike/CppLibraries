#pragma once

namespace ERROR_HANDLING
{
    /// Displays an error message in a pop-up dialog box.
    /// Provides a way to provide feedback to users, even in release mode.
    class ErrorMessageBox
    {
    public:
        static void DisplayLastErrorMessage();
        static void Display(const char* const error_message);
    };
}
