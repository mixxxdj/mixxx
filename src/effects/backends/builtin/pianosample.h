#pragma once

#include "audio/types.h"
#include "util/span.h"

std::span<const CSAMPLE> pianoSampleForSampleRate(mixxx::audio::SampleRate rate);
