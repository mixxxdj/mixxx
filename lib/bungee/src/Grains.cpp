// Copyright (C) 2020-2026 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#include "Grains.h"

#include "log2.h"

namespace Bungee {

bool Grains::flushed() const
{
	for (auto &grain : vector)
		if (!std::isnan(grain->request.position))
			return false;
	return true;
}

void Grains::prepare()
{
	const auto log2TransformLength = (*this)[0].log2TransformLength;

	// Only grains 0 and 1 need these buffers.
	Fourier::resize<true>(log2TransformLength, 1, (*this)[0].phase);
	Fourier::resize<true>(log2TransformLength, 1, (*this)[1].phase);
	Fourier::resize<true>(log2TransformLength, 1, (*this)[0].energy);
	Fourier::resize<true>(log2TransformLength, 1, (*this)[1].energy);
	Fourier::resize<true>(log2TransformLength, 1, (*this)[0].rotation);
	Fourier::resize<true>(log2TransformLength, 1, (*this)[1].rotation);
	(*this)[0].partials.reserve(1 << log2TransformLength);
	(*this)[1].partials.reserve(1 << log2TransformLength);
}

void Grains::rotate()
{
	std::unique_ptr<Grain> grain = std::move(vector.back());
	for (auto i = vector.size() - 1; i > 0; --i)
		vector[i] = std::move(vector[i - 1]);
	vector.front() = std::move(grain);

	// Only grains 0 and 1 need these buffers. Swap them around to avoid reallocating.
	std::swap((*this)[0].phase, (*this)[2].phase);
	std::swap((*this)[0].energy, (*this)[2].energy);
	std::swap((*this)[0].rotation, (*this)[2].rotation);
	std::swap((*this)[0].partials, (*this)[2].partials);
}

} // namespace Bungee
