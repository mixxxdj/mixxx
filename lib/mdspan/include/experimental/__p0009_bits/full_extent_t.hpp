//@HEADER
// ************************************************************************
//
//                        Kokkos v. 4.0
//       Copyright (2022) National Technology & Engineering
//               Solutions of Sandia, LLC (NTESS).
//
// Under the terms of Contract DE-NA0003525 with NTESS,
// the U.S. Government retains certain rights in this software.
//
// Part of Kokkos, under the Apache License v2.0 with LLVM Exceptions.
// See https://kokkos.org/LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//@HEADER
#pragma once

#include "macros.hpp"

namespace MDSPAN_IMPL_STANDARD_NAMESPACE {

struct full_extent_t { explicit full_extent_t() = default; };

_MDSPAN_INLINE_VARIABLE constexpr auto full_extent = full_extent_t{ };

} // namespace MDSPAN_IMPL_STANDARD_NAMESPACE
