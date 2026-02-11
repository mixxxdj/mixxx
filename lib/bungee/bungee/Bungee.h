// Copyright (C) 2020-2026 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

/**
 * @file Bungee.h
 * @brief Main API header for the Bungee audio stretcher library.
 *
 * This header provides both a C++ API and an equivalent C API for the Bungee audio stretcher.
 * Bungee is designed as a C++ library, and users should prefer the C++ API if they are able to use
 * it. Either way, there is only C linkage to the underlying library for maximum compability.
 */

#pragma once

#include "Modes.h"

#ifndef BUNGEE_VISIBILITY
#	define BUNGEE_VISIBILITY
#endif

#ifdef __cplusplus
#	include <cstdint>
#	define BUNGEE_PREFIX extern "C" BUNGEE_VISIBILITY const Bungee::Functions
namespace Bungee {
#else
#	include <stdint.h>
#	define BUNGEE_PREFIX BUNGEE_VISIBILITY const struct Functions
#	define bool char
#endif

/**
 * @brief An object of type Request is passed to the audio stretcher every time an audio grain is processed.
 */
struct Request
{
	/**
	 * @brief Frame-offset within the input audio of the centre-point of the current audio grain.
	 * @details NaN signifies an invalid grain that produces no audio output and may be used for flushing.
	 */
	double position;

	/**
	 * @brief Output audio speed. A value of 1 means speed should be unchanged relative to the input audio.
	 * @details Used by Stretcher's internal algorithms only when it's not possible to determine speed by
	 * subtracting Request::position of previous grain from Request::position of current grain.
	 */
	double speed;

	/**
	 * @brief Adjustment as a frequency multiplier with a value of 1 meaning no pitch adjustment.
	 */
	double pitch;

	/**
	 * @brief Set to have the stretcher forget all previous grains and restart on this grain.
	 */
	bool reset;

	/**
	 * @brief How resampling should be applied to this grain.
	 */
	enum ResampleMode resampleMode;
};

/**
 * @brief Information to describe a chunk of audio that the audio stretcher requires as input for the current grain.
 * @details Input chunks of consecutive grains often overlap and are usually centred on the grain's Request::position.
 */
struct InputChunk
{
	/**
	 * @brief Frame offset relative to the start of the audio track (first sample of chunk).
	 */
	int begin;

	/**
	 * @brief Frame offset relative to the start of the audio track (sample after the last sample of chunk).
	 */
	int end;
};

/**
 * @brief Describes a chunk of audio output.
 * @details Output chunks do not overlap and should be appended for seamless playback.
 */
struct OutputChunk
{
	/**
	 * @brief Audio output data, not aligned and not interleaved.
	 */
	float *data;

	/**
	 * @brief Number of frames in the output chunk.
	 */
	int frameCount;

	/**
	 * @brief nth audio channel audio starts at data[n * channelStride].
	 */
	intptr_t channelStride;

#ifdef __cplusplus
	static constexpr int begin = 0, end = 1;
#endif
	/**
	 * @brief request[0] corresponds to the first frame of data, request[1] corresponds to the frame after the last frame of data.
	 */
	const struct Request *request[2];
};

/**
 * @brief Stretcher audio sample rates, in Hz.
 */
struct SampleRates
{
	/**
	 * @brief Input sample rate in Hz.
	 */
	int input;

	/**
	 * @brief Output sample rate in Hz.
	 */
	int output;
};

/**
 * @brief C API function table for the Bungee stretcher.
 * @details This struct is not part of the C++ API. It is necessary here to facilitate extern "C" linkage to shared libraries.
 * Users of the C API may use this struct to access the functions of the Bungee stretcher.
 */
struct Functions
{
	/** @brief Returns the edition name. */
	const char *(*edition)(void);

	/** @brief Returns the version string. */
	const char *(*version)(void);

	/** @brief Creates a new stretcher instance. */
	void *(*create)(struct SampleRates sampleRates, int channelCount, int log2SynthesisHopAdjust);

	/** @brief Destroys a stretcher instance. */
	void (*destroy)(void *implementation);

	/** @brief Enables or disables instrumentation. */
	void (*enableInstrumentation)(void *implementation, int enable);

	/** @brief Returns the maximum input frame count. */
	int (*maxInputFrameCount)(const void *implementation);

	/** @brief Adjusts request for preroll. */
	void (*preroll)(const void *implementation, struct Request *request);

	/** @brief Prepares request for the next grain. */
	void (*next)(const void *implementation, struct Request *request);

	/** @brief Specifies the input chunk for a grain. */
	struct InputChunk (*specifyGrain)(void *implementation, const struct Request *request, double bufferStartPosition);

	/** @brief Analyses the grain. */
	void (*analyseGrain)(void *implementation, const float *data, intptr_t channelStride, int muteFrameCountHead, int muteFrameCountTail);

	/** @brief Synthesises the grain. */
	void (*synthesiseGrain)(void *implementation, struct OutputChunk *outputChunk);

	/** @brief Returns true if the stretcher is flushed. */
	bool (*isFlushed)(const void *implementation);
};

#ifdef __cplusplus
}
#else
#	undef bool
#endif

/**
 * @brief Returns the function table for the open-source Bungee Basic edition.
 * @details Not part of the C++ API; exists primarily to facilitate extern "C" linkage to Bungee's shared libraries.
 */
BUNGEE_PREFIX *getFunctionsBungeeBasic(void);

/**
 * @brief Returns the function table for the commercial Bungee Pro edition.
 * @details Not part of the C++ API; exists primarily to facilitate extern "C" linkage to Bungee's shared libraries.
 */
BUNGEE_PREFIX *getFunctionsBungeePro(void);

#ifdef __cplusplus
namespace Bungee {

/**
 * @brief Stretcher<Basic> is the open-source implementation contained in this repository.
 */
struct Basic
{
	static constexpr auto getFunctions = &getFunctionsBungeeBasic;
};

/**
 * @brief Stretcher<Pro> is an enhanced and optimised implementation available under commercial license.
 */
struct Pro
{
	static constexpr auto getFunctions = &getFunctionsBungeePro;
};

/**
 * @brief Bungee audio stretcher class template.
 *
 * This is the main class users should instantiate to perform time-stretching and pitch-shifting operations.
 *
 * @tparam Edition The edition type (e.g., Basic or Pro) specifying the implementation.
 */
template <class Edition>
struct Stretcher
{
	/**
	 * @brief Returns the edition name (e.g., "Pro" or "Basic").
	 * @return The edition name string.
	 */
	static inline const char *edition()
	{
		return Edition::getFunctions()->edition();
	}

	/**
	 * @brief Returns the release version string (e.g., "1.2.3").
	 * @return The version string.
	 */
	static inline const char *version()
	{
		return Edition::getFunctions()->version();
	}

	/**
	 * @brief Constructs a new Stretcher instance.
	 *
	 * @param sampleRates The input and output sample rates.
	 * @param channelCount Number of audio channels.
	 * @param log2SynthesisHopAdjust Granularity adjustment. Non-zero values are likely to result in degraded audio
	 * quality so most users should leave this set to zero.
	 *  -1 doubles granular frequency (lower latency, may help weak transients),
	 * +1 halves granular frequency (may benefit dense tones).
	 * Values other than -1, 0, and +1 are unsupported.
	 */
	inline Stretcher(SampleRates sampleRates, int channelCount, int log2SynthesisHopAdjust = 0) :
		functions(Edition::getFunctions()),
		state(functions->create(sampleRates, channelCount, log2SynthesisHopAdjust))
	{
	}

	/**
	 * @brief Destructor. Destroys the stretcher instance and releases resources.
	 */
	inline ~Stretcher()
	{
		functions->destroy(state);
	}

	/**
	 * @brief Enables or disables verbose diagnostics and instrumentation.
	 *
	 * @param enable Set to true to enable diagnostics, false to disable.
	 * @note Diagnostics are reported to the system log file on iOS, Mac, and Android, or to stderr on other platforms.
	 */
	inline void enableInstrumentation(bool enable)
	{
		functions->enableInstrumentation(state, enable);
	}

	/**
	 * @brief Returns the largest number of frames that might be requested by specifyGrain().
	 *
	 * This helps the caller allocate large enough buffers. InputChunk::frameCount() will not exceed this value.
	 * @return Maximum input frame count.
	 */
	inline int maxInputFrameCount() const
	{
		return functions->maxInputFrameCount(state);
	}

	/**
	 * @brief Adjusts the request for preroll.
	 *
	 * This function adjusts request.position so the stretcher has a run-in of a few grains before the requested position.
	 * Without preroll, the first milliseconds of audio might sound weak or initial transients might be lost.
	 * @param request The request to adjust.
	 */
	inline void preroll(Request &request) const
	{
		functions->preroll(state, &request);
	}

	/**
	 * @brief Prepares the request for the next grain.
	 *
	 * Typically called within a granular loop where playback at constant request.speed is desired.
	 * @param request The request to update for the next grain.
	 */
	inline void next(Request &request) const
	{
		functions->next(state, &request);
	}

	/**
	 * @brief Specifies a grain of audio and computes the necessary segment of input audio.
	 *
	 * After calling this function, call analyseGrain().
	 * @param request The request describing the grain.
	 * @param bufferStartPosition The start position of the buffer (default 0).
	 * @return The input chunk required for the grain.
	 */
	inline InputChunk specifyGrain(const Request &request, double bufferStartPosition = 0.)
	{
		return functions->specifyGrain(state, &request, bufferStartPosition);
	}

	/**
	 * @brief Begins processing the grain.
	 *
	 * The audio data should correspond to the range specified by specifyGrain's return value.
	 * After calling this function, call synthesiseGrain().
	 * @param data Pointer to input audio data.
	 * @param channelStride Stride between channels in the data buffer.
	 * @param muteFrameCountHead Number of unavailable frames at the start (default 0).
	 * @param muteFrameCountTail Number of unavailable frames at the end (default 0).
	 * @note Mute frames are replaced with zero-valued frames.
	 */
	inline void analyseGrain(const float *data, intptr_t channelStride, int muteFrameCountHead = 0, int muteFrameCountTail = 0)
	{
		functions->analyseGrain(state, data, channelStride, muteFrameCountHead, muteFrameCountTail);
	}

	/**
	 * @brief Completes processing of the grain of audio previously set up with specifyGrain() and analyseGrain().
	 *
	 * @param outputChunk The output chunk to fill with synthesised audio.
	 */
	inline void synthesiseGrain(OutputChunk &outputChunk)
	{
		functions->synthesiseGrain(state, &outputChunk);
	}

	/**
	 * @brief Returns true if every grain in the stretcher's pipeline is invalid (its Request::position was NaN).
	 * @return True if the stretcher is flushed, false otherwise.
	 */
	inline bool isFlushed() const
	{
		return functions->isFlushed(state);
	}

	/**
	 * @brief Pointer to the function table for the stretcher implementation.
	 */
	const Functions *const functions;

	/**
	 * @brief Pointer to the internal stretcher state.
	 */
	void *const state;
};

} // namespace Bungee
#endif
