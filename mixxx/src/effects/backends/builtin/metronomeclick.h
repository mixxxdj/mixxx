#pragma once

#include "audio/types.h"
#include "util/span.h"

std::span<const CSAMPLE> clickForSampleRate(mixxx::audio::SampleRate rate);
