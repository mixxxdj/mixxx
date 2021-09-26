#pragma once

// Maximum buffer length to each EngineObject::process call.
//TODO: Replace this with mixxx::AudioParameters::bufferSize()
constexpr unsigned int MAX_BUFFER_LEN = 160000;

constexpr int kMaxNumberOfDecks = 4;
