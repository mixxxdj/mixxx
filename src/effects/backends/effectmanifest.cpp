#include "effects/backends/effectmanifest.h"

QDebug operator<<(QDebug dbg, const EffectManifest& manifest) {
    return dbg.maybeSpace() << QString("EffectManifest(%1)").arg(manifest.id());
}
