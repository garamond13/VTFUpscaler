#pragma once

#include <cassert>

#ifdef NDEBUG
#define vtfu_assert(keep, discard_if_ndebug) keep
#else
#define vtfu_assert(keep, discard_if_ndebug) (assert(keep discard_if_ndebug))
#endif