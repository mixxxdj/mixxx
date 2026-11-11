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

#ifndef MDSPAN_HPP_
#define MDSPAN_HPP_

#ifndef MDSPAN_IMPL_STANDARD_NAMESPACE
  #define MDSPAN_IMPL_STANDARD_NAMESPACE Kokkos
#endif

#ifndef MDSPAN_IMPL_PROPOSED_NAMESPACE
  #define MDSPAN_IMPL_PROPOSED_NAMESPACE Experimental
#endif

#include "../experimental/__p0009_bits/default_accessor.hpp"
#include "../experimental/__p0009_bits/full_extent_t.hpp"
#include "../experimental/__p0009_bits/mdspan.hpp"
#include "../experimental/__p0009_bits/dynamic_extent.hpp"
#include "../experimental/__p0009_bits/extents.hpp"
#include "../experimental/__p0009_bits/layout_stride.hpp"
#include "../experimental/__p0009_bits/layout_left.hpp"
#include "../experimental/__p0009_bits/layout_right.hpp"
#include "../experimental/__p0009_bits/macros.hpp"
#if MDSPAN_HAS_CXX_17
#include "../experimental/__p2630_bits/submdspan.hpp"
#endif

#endif // MDSPAN_HPP_
