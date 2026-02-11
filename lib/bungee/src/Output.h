// Copyright (C) 2020-2026 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "Fourier.h"
#include "Resample.h"
#include "Window.h"

#include "bungee/Bungee.h"

#include <Eigen/Core>

#include <initializer_list>

namespace Bungee {

struct Grains;

struct Output
{
	Eigen::ArrayXf synthesisWindow;
	Eigen::ArrayXXf inverseTransformed;
	Eigen::ArrayXXf bufferResampled;
	Resample::Internal lappedSynthesisBuffer;

	Output(Fourier::Transforms &transforms, int log2SynthesisHop, int channelCount, int maxOutputChunkSize, float windowGain, std::initializer_list<float> windowCoefficients);

	void applySynthesisWindow(int log2SynthesisHop, Grains &grains, const Eigen::Ref<const Eigen::ArrayXf> &window);

	OutputChunk resample(Resample::Operation resampleOperationBegin, Resample::Operation resampleOperationEnd);
};

} // namespace Bungee
