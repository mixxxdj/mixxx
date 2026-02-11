// Copyright (C) 2020-2026 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#include "Partials.h"

namespace Bungee::Partials {

void enumerate(std::vector<Partial> &partials, int n, Eigen::Ref<Eigen::ArrayX<float>> energy)
{
	float undo[] = {-1.f, 0.f};
	std::swap(energy[n], undo[0]);
	std::swap(energy[n + 1], undo[1]);

	partials.resize(partials.capacity());

	int count = 0;
	int m = 1;
	do
	{
		while ((energy[m] < energy[m + 1]))
			m++;
		partials[count].peak = m++;

		while (!(energy[m] < energy[m + 1]))
			m++;
		partials[count++].end = m++;

	} while (m < n + 1);

	partials.resize(count);

	BUNGEE_ASSERT1(partials.back().end == n);

	std::swap(energy[n], undo[0]);
	std::swap(energy[n + 1], undo[1]);
}

inline void suppressPartial(std::vector<Partial> &partials, int i, const Eigen::Ref<const Eigen::ArrayX<float>> energy)
{
	if (energy[partials[i - 1].end] > energy[partials[i].end])
		partials[i - 1].end = partials[i].end;
	else
		partials[i].end = partials[i - 1].end;
}

void suppressTransientPartials(std::vector<Partial> &partials, const Eigen::Ref<const Eigen::ArrayX<float>> energy, const Eigen::Ref<const Eigen::ArrayX<float>> previousEnergy)
{
	int strongestPartialIndex = 0;
	for (int i = 1; i < partials.size(); ++i)
		if (energy[partials[i].peak] > energy[partials[strongestPartialIndex].peak])
			strongestPartialIndex = i;

	for (int i = 1; i < partials.size() - 1; ++i)
		if (i != strongestPartialIndex)
		{
			constexpr auto k = 1.5f; // fudge: lower constant helps transients, higher helps tones
			if (energy[partials[i].peak] > k * previousEnergy[partials[i].peak])
				suppressPartial(partials, i, energy);
		}
}

} // namespace Bungee::Partials
