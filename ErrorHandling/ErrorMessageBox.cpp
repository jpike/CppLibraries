#ifdef _WIN32

#include <strsafe.h>
#include <Windows.h>
#include "ErrorHandling/ErrorMessageBox.h"

namespace ERROR_HANDLING
{
    /// Displays the last error message from Windows in a pop-up error dialog box.
    void ErrorMessageBox::DisplayLastErrorMessage()
    {
        // GET THE LAST ERROR CODE.
        DWORD last_error_code = GetLastError();

        // GET THE MESSAGE ASSOCIATED WITH THE ERROR CODE.
        constexpr LPCVOID NO_MESSAGE_SOURCE_SINCE_RETRIEVED_FROM_SYSTEM = nullptr;
        constexpr DWORD NEUTRAL_LANGUAGE_ID = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);
        LPTSTR system_error_message = nullptr;
        constexpr DWORD SYSTEM_WILL_ALLOCATE_SIZE = 0;
        constexpr va_list* NO_ARGUMENTS = nullptr;
        DWORD format_message_return_code = FormatMessage(
            // Ignoring inserts allows us to custom insert the error code in the message later.
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NO_MESSAGE_SOURCE_SINCE_RETRIEVED_FROM_SYSTEM,
            last_error_code,
            NEUTRAL_LANGUAGE_ID,
            system_error_message,
            SYSTEM_WILL_ALLOCATE_SIZE,
            NO_ARGUMENTS);
        if (!format_message_return_code)
        {
            // There is not much else we can do.
            return;
        }

        // ALLOCATE MEMORY FOR DISPLAY THE MESSAGE IN A MESSAGE BOX.
        // Some additional space is allocated for extra text (plus the error code) we are inserting.
        constexpr SIZE_T ADDITIONAL_BUFFER_SPACE_IN_CHARACTERS = 40;
        SIZE_T system_error_message_length_in_characters = lstrlen(system_error_message);
        SIZE_T message_box_text_buffer_length_in_characters = system_error_message_length_in_characters + ADDITIONAL_BUFFER_SPACE_IN_CHARACTERS;
        SIZE_T message_box_text_buffer_size_in_bytes = message_box_text_buffer_length_in_characters * sizeof(TCHAR);
        LPVOID message_box_text_buffer = (LPVOID)LocalAlloc(LMEM_ZEROINIT, message_box_text_buffer_size_in_bytes); 
        if (!message_box_text_buffer)
        {
            // There is not much else we can do.
            // Free memory first before returning.
            LocalFree(system_error_message);
            return;
        }

        // PRINT THE ERROR CODE AND MESSAGE TO A STRING.
        HRESULT string_formatting_result = StringCchPrintf(
            (LPTSTR)message_box_text_buffer, 
            message_box_text_buffer_length_in_characters,
            TEXT("Failed with error %d: %s"), 
            last_error_code, 
            system_error_message); 
        bool string_formatting_failed = FAILED(string_formatting_result);
        if (string_formatting_failed)
        {
            // There is not much else we can do.
            // Free memory first before returning.
            LocalFree(message_box_text_buffer);
            LocalFree(system_error_message);
            return;
        }

        // DISPLAY THE ERROR MESSAGE IN A DIALOG BOX.
        constexpr HWND NO_OWNER_WINDOW = NULL;
        MessageBox(
            NO_OWNER_WINDOW,
            (LPCTSTR)message_box_text_buffer,
            TEXT("Error - Please report to developers"),
            MB_ICONERROR | MB_OK);

        // FREE MEMORY.
        LocalFree(message_box_text_buffer);
        LocalFree(system_error_message);
    }

    /// Displays the provided error message in an error box.
    /// @param[in] error_message - The error message to display.
    void ErrorMessageBox::Display(const char* const error_message)
    {
        constexpr HWND NO_OWNER_WINDOW = NULL;
        MessageBox(
            NO_OWNER_WINDOW,
            error_message,
            TEXT("Error - Please report to developers"),
            MB_ICONERROR | MB_OK);
    }
}

#endif

