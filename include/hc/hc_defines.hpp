//===----------------------------------------------------------------------===//
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#pragma once

#include <cstdint>

namespace hc
{
    // TODO: assess why this exists.
    typedef _Float16 half;
}

//
// work-item related builtin functions
//
extern "C"
__attribute__((const))
std::uint32_t hc_get_grid_size(std::uint32_t n) [[hc]];
extern "C"
__attribute__((const))
std::uint32_t hc_get_workitem_absolute_id(std::uint32_t n) [[hc]];
extern "C"
__attribute__((const))
std::uint32_t hc_get_group_size(std::uint32_t n) [[hc]];
extern "C"
__attribute__((const))
std::uint32_t hc_get_workitem_id(std::uint32_t n) [[hc]];
extern "C"
__attribute__((const))
std::uint32_t hc_get_num_groups(std::uint32_t n) [[hc]];
extern "C"
__attribute__((const))
std::uint32_t hc_get_group_id(std::uint32_t n) [[hc]];

// TODO: this should be implemented as a keyword (+possibly storage class).
#define tile_static __attribute__((tile_static))

extern "C"
__attribute__((noduplicate, nothrow))
void hc_barrier(unsigned int n) [[hc]];

#ifndef CLK_LOCAL_MEM_FENCE
    #define CLK_LOCAL_MEM_FENCE (1)
#endif

#ifndef CLK_GLOBAL_MEM_FENCE
    #define CLK_GLOBAL_MEM_FENCE (2)
#endif

// Valid values for__hcc_backend__ to indicate the
// compiler backend
#define HCC_BACKEND_AMDGPU (1)