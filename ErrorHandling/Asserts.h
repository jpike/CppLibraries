#pragma once

#include <cassert>

#define ASSERT_THEN_IF(condition) assert((condition)); if ((condition))

#define ASSERT_THEN_IF_NOT(condition) assert((condition)); if (!(condition))
