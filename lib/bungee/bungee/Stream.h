// Copyright (C) 2020-2026 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#include "Bungee.h"

#include <algorithm>
#include <cassert>
#include <span>

#pragma once

namespace Bungee {

/**
 * @brief Wrapper for Bungee::Stretch<> for forward playback streaming applications.
 *
 * Provides an easy-to-use API for streaming audio using Bungee's stretcher.
 * Example usage can be found in ../cmd/main.cpp.
 *
 * @tparam Implementation The implementation type for the stretcher.
 */
template <class Implementation>
class Stream
{
	class InputBuffer
	{
		const int channelStride;
		const int channelCount;
		std::vector<float> buffer;
		int begin = 0;
		int end = 0;

	public:
		Stretcher<Implementation> &stretcher;
		InputChunk inputChunk{};

		InputBuffer(Stretcher<Implementation> &stretcher, int maxFrameCount, int channelCount) :
			channelStride(maxFrameCount),
			channelCount(channelCount),
			buffer(channelStride * channelCount),
			stretcher(stretcher)
		{
		}

		void append(int inputFrameCount, const float *const *inputPointers)
		{
			int discard = 0;

			if (inputChunk.begin < end)
			{
				if (begin < inputChunk.begin)
				{
					for (int x = 0; x < (int)buffer.size(); x += channelStride)
						std::move(
							&buffer[x + inputChunk.begin - begin],
							&buffer[x + end - begin],
							&buffer[x]);
					begin = inputChunk.begin;
				}
			}
			else
			{
				discard = std::min(inputChunk.begin - begin, inputFrameCount);
				begin = end;
			}

			for (int c = 0; c < channelCount; ++c)
				if (inputPointers)
					std::copy(
						&inputPointers[c][discard],
						&inputPointers[c][inputFrameCount],
						&buffer[(end - begin) + c * channelStride]);
				else
					std::fill(
						&buffer[(end - begin) + c * channelStride],
						&buffer[(end - begin) + c * channelStride + (inputFrameCount - discard)],
						0.f);
			begin += discard;
			end += inputFrameCount;
			assert(end >= begin);
			assert(end - begin <= channelStride);
		}

		auto endPosition() const
		{
			return end;
		}

		void analyseGrain() const
		{
			const int muteHead = begin - inputChunk.begin;
			const int muteTail = inputChunk.end - end;
			assert(muteHead >= (inputChunk.end - inputChunk.begin) || muteTail <= 0);
			stretcher.analyseGrain(buffer.data() - muteHead, channelStride, muteHead, muteTail);
		}
	};

	const int channelCount;

	InputBuffer inputBuffer;

	Request request{};

	OutputChunk outputChunk{};
	int outputChunkConsumed = 0;

	double framesNeeded = 0.;

public:
	/**
	 * @brief Construct a Stream.
	 * @param stretcher Reference to the stretcher
	 * @param maxInputFrameCount Maximum number of input frames per process
	 * @param channelCount Number of channels
	 */
	Stream(Stretcher<Implementation> &stretcher, int maxInputFrameCount, int channelCount) :
		channelCount(channelCount),
		inputBuffer(stretcher, stretcher.maxInputFrameCount() + maxInputFrameCount, channelCount)
	{
		request.position = std::numeric_limits<double>::quiet_NaN();
	}

	/**
	 * @brief Process a segment of audio.
	 *
	 * Renders output samples to outputPointers. The number of samples is set by dithering to floor or ceil of outputFrameCount.
	 *
	 * @param inputPointers Array of pointers to input audio (per channel), nullptr for mute input
	 * @param outputPointers Array of pointers to output audio (per channel)
	 * @param inputFrameCount Number of input audio samples to process
	 * @param outputFrameCount Number of output audio samples required (may be fractional)
	 * @param pitch Audio pitch shift (see Request::pitch)
	 * @return Number of output samples rendered
	 */
	int process(
		const float *const *inputPointers,
		float *const *outputPointers,
		int inputFrameCount,
		double outputFrameCount,
		double pitch = 1.)
	{
		inputBuffer.append(inputFrameCount, inputPointers);

		request.speed = inputFrameCount / outputFrameCount;
		request.pitch = pitch;

		framesNeeded += outputFrameCount;

		int frameCounter = 0;
		for (bool processGrain = false; frameCounter != std::round(framesNeeded); processGrain = true)
		{
			if (processGrain)
			{
				if (!std::isnan(request.position))
				{
					inputBuffer.analyseGrain();
					inputBuffer.stretcher.synthesiseGrain(outputChunk);
					outputChunkConsumed = 0;
				}

				[[maybe_unused]] const double proportionRemaining = 1. - frameCounter / std::round(outputFrameCount);
				const double proportionRemainingDenominator = std::round(outputFrameCount);
				const double proportionRemainingNumerator = proportionRemainingDenominator - frameCounter;

				const auto position = inputBuffer.endPosition() - inputBuffer.stretcher.maxInputFrameCount() / 2 - inputFrameCount * proportionRemainingNumerator / proportionRemainingDenominator;
				request.reset = !(position > request.position);
				request.position = position;
				inputBuffer.inputChunk = inputBuffer.stretcher.specifyGrain(request);
			}

			if (outputChunk.request[0] && !std::isnan(outputChunk.request[0]->position))
			{
				const int need = std::round(framesNeeded) - frameCounter;
				const int available = outputChunk.frameCount - outputChunkConsumed;
				const int n = std::min(need, available);

				for (int c = 0; c < channelCount; ++c)
					std::copy(
						outputChunk.data + outputChunkConsumed + c * outputChunk.channelStride,
						outputChunk.data + outputChunkConsumed + c * outputChunk.channelStride + n,
						outputPointers[c] + frameCounter);

				frameCounter += n;
				outputChunkConsumed += n;
			}
		}

		assert(frameCounter == std::floor(outputFrameCount) || frameCounter == std::ceil(outputFrameCount));
		framesNeeded -= frameCounter;
		return frameCounter;
	};

	/**
	 * @brief Get the current position in the input stream.
	 * @return Sum of inputFrameCount over all process() calls
	 */
	int inputPosition() const
	{
		return inputBuffer.endPosition();
	}

	/**
	 * @brief Get the current output position in terms of input frames.
	 * @return Output position (input frame units)
	 */
	double outputPosition() const
	{
		return outputChunk.request[0]->position + outputChunkConsumed * (outputChunk.request[1]->position - outputChunk.request[0]->position) / outputChunk.frameCount;
	}

	/**
	 * @brief Get the latency due to the stretcher.
	 * @return Latency in input frames
	 */
	double latency() const
	{
		return inputPosition() - outputPosition();
	}
};

} // namespace Bungee