// Copyright (C) 2020-2026 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "Grain.h"

#include <memory>
#include <vector>

namespace Bungee {

struct Grains
{
	std::vector<std::unique_ptr<Grain>> vector;

	Grains(size_t n) :
		vector(n)
	{
	}

	void prepare();

	void rotate();

	bool flushed() const;

	inline Grain &operator[](size_t i)
	{
		return *vector[i];
	}
};

} // namespace Bungee
