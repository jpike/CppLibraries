#pragma once

#include <cassert>
#include <Windows.h>

#define ASSERT_THEN_IF(condition) assert((condition)); if ((condition))

#define ASSERT_THEN_IF_NOT(condition) assert((condition)); if (!(condition))

/// @param[in]  result - The Windows HRESULT to check.
#define ASSERT_WINDOWS_RESULT_SUCCESS_THEN_IF_FAILED(result) assert(SUCCEEDED(result)); if (!SUCCEEDED(result))
