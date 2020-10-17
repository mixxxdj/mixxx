#pragma once

// WARNING: These functions are for internal use to assist the migration
// from sample positions to frame positions in beat related operations.
// These should only be used to convert between legacy Control Object data
// which is a double value for sample position and frame position which is
// an object of the class mixxx::FramePos
// The legacy use of samples for positions assumed interleaved stereo buffers.

// This file can be removed when Control Objects are entirely migrated
// to using frames instead of samples.

// Side note: Sample rate directly corresponds to frames not samples.
// For example, a sample rate of 44100 Hz in a stereo track means there are
// 44100 audio frames and 88200 audio samples (44100 for each channel).

/// Convert sample position (double) to frame position (mixxx::FramePos)
inline mixxx::FramePos samplePosToFramePos(double samplePos) {
    return mixxx::FramePos(samplePos / mixxx::kEngineChannelCount);
}

/// Convert frame position (mixxx::FramePos) to sample position (double)
inline double framePosToSamplePos(mixxx::FramePos framePos) {
    return framePos.getValue() * mixxx::kEngineChannelCount;
}

/// Convert sample count (double) to frame count (double aka mixxx::FrameDiff_t)
inline mixxx::FrameDiff_t samplesToFrames(double samples) {
    return samples / mixxx::kEngineChannelCount;
}

/// Convert frame count (double aka mixxx::FrameDiff_t) to sample count (double)
inline double framesToSamples(mixxx::FrameDiff_t frames) {
    return frames * mixxx::kEngineChannelCount;
}
