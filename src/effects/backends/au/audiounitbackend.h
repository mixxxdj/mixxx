#pragma once

#include <QList>
#include <QString>

#include "effects/backends/effectprocessor.h"
#include "effects/backends/effectsbackend.h"
#include "effects/defs.h"

EffectsBackendPointer createAudioUnitBackend();
