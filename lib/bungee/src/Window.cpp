// Copyright (C) 2020-2026 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#include "Window.h"
#include "Assert.h"
#include "Fourier.h"

#include <Eigen/Core>

#include <cmath>

namespace Bungee::Window {

Eigen::ArrayXf fromFrequencyDomainCoefficients(Fourier::Transforms &transforms, int log2Size, float gain, std::initializer_list<float> coefficients)
{
	Eigen::ArrayXcf frequencyDomain(Fourier::binCount(log2Size));

	std::size_t row = 0;
	for (auto c : coefficients)
		if (row < frequencyDomain.rows())
			frequencyDomain.coeffRef(row++) = c * gain;

	frequencyDomain.bottomRows(frequencyDomain.rows() - row).setZero();

	Eigen::ArrayXf window(Fourier::transformLength(log2Size));
	transforms.prepareInverse(log2Size);
	transforms.inverse(log2Size, window, frequencyDomain);
	return window;
}

} // namespace Bungee::Window
