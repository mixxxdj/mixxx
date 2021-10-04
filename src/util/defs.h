#pragma once

#include "preferences/configobject.h"

// Maximum buffer length to each EngineObject::process call.
//TODO: Replace this with mixxx::AudioParameters::bufferSize()
constexpr unsigned int MAX_BUFFER_LEN = 160000;

constexpr int kMaxNumberOfDecks = 4;

const ConfigKey kMaximizeLibraryConfigKey = ConfigKey("[Library]", "maximize_library");
