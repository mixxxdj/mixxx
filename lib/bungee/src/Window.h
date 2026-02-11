// Copyright (C) 2020-2026 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "Fourier.h"

#include <initializer_list>

namespace Bungee::Window {

Eigen::ArrayXf fromFrequencyDomainCoefficients(Fourier::Transforms &transforms, int log2Size, float gain, std::initializer_list<float> coefficients);

} // namespace Bungee::Window
