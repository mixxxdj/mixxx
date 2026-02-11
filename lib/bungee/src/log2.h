// Copyright (C) 2020-2026 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "Assert.h"

#include <bit>
#include <type_traits>

namespace Bungee {

template <bool floor = false>
static inline int log2(unsigned x)
{
	BUNGEE_ASSERT1(x > 0);
	BUNGEE_ASSERT1(floor || !(x & (x << 1)));

	int y;
	if constexpr (floor)
		y = std::bit_width(x) - 1;
	else
		y = std::countr_zero(x);

	BUNGEE_ASSERT1(floor ? (1 << y <= x && x < 2 << y) : (x == 1 << y));
	return y;
}

template <int x>
constexpr int log2(std::integral_constant<int, x>)
{
	static_assert(x > 0);
	static_assert(!(x & (x - 1)));

	if constexpr (x == 1)
		return 0;
	else
		return 1 + log2(std::integral_constant<int, x / 2>{});
}

} // namespace Bungee
