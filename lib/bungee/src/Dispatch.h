// Copyright (C) 2020-2026 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <array>

namespace Bungee {

template <class Target, int n>
struct Dispatch
{
	typedef decltype(&Target::template special<0>) FunctionPointer;

	std::array<FunctionPointer, n> table;

	template <class T, int begin = 0, int end = n>
	inline constexpr void populateTable()
	{
		if (end - begin == 1)
		{
			table[begin] = &T::template special<begin>;
		}
		else
		{
			constexpr auto middle = (begin + end) / 2;
			populateTable<T, begin, middle>();
			populateTable<T, middle, end>();
		}
	}

	constexpr Dispatch()
	{
		populateTable<Target>();
	}

	inline auto operator[](int i) const
	{
		return table[i];
	}
};

} // namespace Bungee