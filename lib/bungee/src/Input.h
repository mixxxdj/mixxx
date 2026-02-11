// Copyright (C) 2020-2026 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "Assert.h"
#include "Fourier.h"
#include "Resample.h"

#include <Eigen/Core>

namespace Bungee {

struct Input
{
	Eigen::ArrayXf window;
	Eigen::ArrayXXf windowedInput;
	Eigen::ArrayXXf windowedInputPrevious;
	Resample::Internal resampled;
	float scale;

	Input(int log2SynthesisHop, int channelCount, Fourier::Transforms &transforms);

	// returns transformLength
	int applyAnalysisWindow(const Eigen::Ref<const Eigen::ArrayXXf> &input, int muteFrameCountHead, int muteFrameCountTail);
};

} // namespace Bungee

struct Bungee_InputCheck
{
	Bungee::Input *input;
};
