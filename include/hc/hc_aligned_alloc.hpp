//===----------------------------------------------------------------------===//
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#pragma once

#include <cassert>
#include <cstdlib>
#include <memory>
#include <type_traits>

/** \cond HIDDEN_SYMBOLS */
namespace detail {

inline
constexpr
bool hc_is_alignment(std::size_t value) noexcept
{
    return (value > 0) && ((value & (value - 1)) == 0);
}

inline
void* hc_aligned_alloc(std::size_t alignment, std::size_t size) noexcept
{
    assert(hc_is_alignment(alignment));

    if (alignment < alignof(void*)) {
        alignment = alignof(void*);
    }
    void* memptr = NULL;
    // posix_memalign shall return 0 upon successfully allocate aligned memory
    posix_memalign(&memptr, alignment, size);
    assert(memptr);

    return memptr;
}

inline
void hc_aligned_free(void* ptr) noexcept
{
    if (ptr) {
        std::free(ptr);
    }
}

} // namespace detail
/** \endcond */
